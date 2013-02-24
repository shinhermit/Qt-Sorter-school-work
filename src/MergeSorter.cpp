#include "MergeSorter.hpp"

int MergeSorter::_sigFd[2] = {-1, -1};

MergeSorter::MergeSorter(const QString & execName,
			 const pid_t & ppid,
			 const SortTypes::fd_t & writeChannel,
			 const SortTypes::fd_t & readChannel)
  :_ui( new Ui::SorterView() ),
   _execName( new QString(execName) ),
   _numbers( new QVector<int>() ),
   _ppid(ppid),
   _p_writeChannel(writeChannel),
   _p_readChannel(readChannel),
   _left_results( new QVector<int>() ),
   _right_results( new QVector<int>() ),
   _partsReceived(0)
{
  _ui->setupUi(this);

  _setSigAction();

  //Unix handler to Qt handling
  if ( ::socketpair(AF_UNIX, SOCK_STREAM, 0, _sigFd) == -1 )
    qFatal("::socketpair, in Controler::Controler.");

  _sigNotifier = new QSocketNotifier(_sigFd[0], QSocketNotifier::Read, this);
  connect(_sigNotifier, SIGNAL(activated(int)), this, SLOT(_sigSlot()));

  //fetch data in pipe
  _fetchData(_p_readChannel, *_numbers);

  _updateDisplayedList(*_ui->initialListLineEdit, *_numbers);

  _updateStatus("Sorter ready");

  //lancer le tri au bout d'une seconde
  QTimer::singleShot(1000, this, SLOT(_sortSlot()));
}

MergeSorter::~MergeSorter()
{
  delete _ui;
  delete _execName;
  delete _numbers;
  delete _sigNotifier;
}

void MergeSorter::sighandler(int signum, siginfo_t * info, void * secret)
{
  pid_t emitter = info->si_pid;
  SortTypes::SigCode sigCode = SortUtility::IntToSigCode(info->si_int);

  if( ::write(_sigFd[1], &emitter, sizeof(pid_t)) == -1)
    qFatal("write (1), in Controler::sighandler");

  if( ::write(_sigFd[1], &sigCode, sizeof(SortTypes::SigCode)) == -1)
    qFatal("write (2), in Controler::sighandler");

  if(!secret)
    qDebug() << "Controler::sighandler, received signal " << signum << " with code " << SortUtility::SigCodeToQString(sigCode) << "\n";
}

void MergeSorter::_sigSlot()
{
  _updateStatus("Signal notification received");

  _sigNotifier->setEnabled(false);

  if( ::read(_sigFd[0], &_sigEmitter, sizeof(pid_t)) == -1)
    qFatal("read (1), in Controler::sigSlot");

  if( ::read(_sigFd[0], &_sigCode, sizeof(SortTypes::SigCode)) == -1)
    qFatal("read (2), in Controler::sigSlot");

  switch(_sigCode)
    {
    case SortTypes::RESULTS_RECEIVED: //from father
        QTimer::singleShot(1000, this, SLOT(close()));
      break;

    case SortTypes::RESULTS_SENT: //from child
      _fetchData(_getReadChannel(_sigEmitter), _getResultDestination(_sigEmitter));
      _tell(_sigEmitter, SortTypes::RESULTS_RECEIVED);

      ++_partsReceived;
      if(_partsReceived == 2)
	{
	  _mergeResults();
	  _sendData(_p_writeChannel, *_numbers);
	  _tell(_ppid, SortTypes::RESULTS_SENT);
	}
      break;

    default:
      qFatal("MergeSorter::_sigSlot, received unexpected sigCode");
    }

  _sigNotifier->setEnabled(true);  

  qDebug() << "Received SigCode " << SortUtility::SigCodeToQString(_sigCode) << "from (" << _sigEmitter << ")" << "\n";
}

void MergeSorter::_sortSlot()
{
  if(_numbers->size() > 1)
    {
      _dispatchWork();
    }

  else
    {
      _sendData(_p_writeChannel, *_numbers);
      _tell(_ppid, SortTypes::RESULTS_SENT);
    }
}

void MergeSorter::_validationSlot()
{
  qDebug() << "MergeSorter::_validationSlot, Validation button pressed\n";
}

void MergeSorter::_setSigAction()
{

  if( sigemptyset(&_sigact.sa_mask) == -1)
      qFatal("sigemptyset, in MergeSorter::_setSigAction");

  _sigact.sa_flags = SA_SIGINFO|SA_RESTART;
  _sigact.sa_sigaction = sighandler;

  if( sigaction(SIGUSR1, &_sigact, NULL) == -1)
      qFatal("sigaction, in MergeSorter::_setSigAction");
}

void MergeSorter::_updateStatus(const QString & status)
{
  _ui->statusPlainTextEdit->setPlainText(QString(". ")+status+"\n" + _ui->statusPlainTextEdit->toPlainText());
}

void MergeSorter::_updateDisplayedList(QLineEdit & listUI, const QVector<int> & numbers, const int & begin, const int & end)
{
  QString listString = "";
  int size = numbers.size();

  for(int i = begin; i <= end; ++i)
    {
      listString += QString::number(numbers[i]);

      if(i < size-1)
	listString += ", ";
    }

  listUI.setText( listString );
}

void MergeSorter::_updateDisplayedList(QLineEdit & listUI, const QVector<int> & numbers)
{
  _updateDisplayedList(listUI, numbers, 0, numbers.size()-1);
}

void MergeSorter::_fetchData(const SortTypes::fd_t & readChannel, QVector<int> & destination)
{
  int datalen, data;

  //data length
  if( ::read(readChannel, &datalen, sizeof(int)) == -1 )
      qFatal("read (1), in MergeSorter::_fetchData");

  //actual data
  for(int i=0; i < datalen; ++i)
    {
      if( ::read(readChannel, &data, sizeof(int)) == -1 )
	qFatal("read (2), in MergeSorter::_fetchData");

      destination.push_back(data);
    }
}

void MergeSorter::_sendData(const int & writeChannel, QVector<int> & dataSource, const int & begin, const int & end)
{

  int datalen = end - begin + 1;

  //data length
  if( write(writeChannel, &datalen, sizeof(int)) == -1)
    qFatal("write (1), in MergeSorter::_sendData");

  //actual data
  for(int i=begin; i <= end; ++i)
    {

      if( write(writeChannel, &dataSource[i], sizeof(int)) == -1)
	qFatal("write (2), in MergeSorter::_sendData");
    }

}

void MergeSorter::_sendData(const int & writeChannel, QVector<int> & dataSource)
{
  _sendData(writeChannel, dataSource, 0, dataSource.size()-1);
}

void MergeSorter::_tell(const pid_t & recipient, const SortTypes::SigCode & sigCode)
{
  union sigval message;

  message.sival_int = sigCode;

  if( ::sigqueue(recipient, SIGUSR1, message) == -1)
    qFatal("sigqueue, in MergSorter::_tell");

  _updateStatus(QString("Signal ")+SortUtility::SigCodeToQString(sigCode)+" sent" );
}

void MergeSorter::_dispatchWork()
{
  int debut = 0;
  int fin = _numbers->size() - 1;
  int milieu = (debut + fin)/2;

  _createSorter(_lpid, _l_writeChannel, _l_readChannel, debut, milieu);
  _createSorter(_rpid, _r_writeChannel, _r_readChannel, milieu+1, fin);
}

void MergeSorter::_createSorter(pid_t & getCreatedPid, int & getWriteChannel, int & getReadChannel, const int & dataBegin, const int & dataEnd)
{
  int dad_writes[2];
  int son_writes[2];
  pid_t control_pid, pgid;

  control_pid = getpid();
  pgid = getpgid(control_pid);

  if( pipe(dad_writes) == -1 || pipe(son_writes) == -1  )
    qFatal("pipe, in Controler::_createSorter");

  getWriteChannel = dad_writes[1];
  getReadChannel = son_writes[0];

  //make list of numbers available for Sorter
  _sendData(dad_writes[1], *_numbers, dataBegin, dataEnd);

  switch( getCreatedPid = fork() )
    {
    case -1:
      qFatal("fork, in MergeSorter::_createSorter");

    case 0:

      ::close(dad_writes[1]);
      ::close(son_writes[0]);

      setpgid(getpid(), pgid);

      if(
	 execl(_execName->toAscii(), _execName->toAscii(),
	       "-p",
	       "sorter",
	       "--control",
	       SortUtility::itos(control_pid).c_str(),
	       "--write-channel",
	       SortUtility::itos(son_writes[1]).c_str(),
	       "--read-channel",
	       SortUtility::itos(dad_writes[0]).c_str(),
	       NULL
	       ) == -1 )
	qFatal("execl, in MergeSorter::_createSorter");

    default:
      ::close(dad_writes[0]);
      ::close(son_writes[1]);

    }

  qDebug() << "write Channnels --- Sorter (" << control_pid << "): " << dad_writes[1] << ", Sorter (" << getCreatedPid <<"): " << son_writes[1] << "\n";
  qDebug() << "read Channnels --- Sorter (" << control_pid << "): " << son_writes[0] << ", Sorter (" << getCreatedPid <<"): " << dad_writes[0] << "\n";

  _updateStatus("Sorter view created" );
}

int & MergeSorter::_getReadChannel(const pid_t & sender)
{
  if(sender == _lpid)
    return _l_readChannel;

  else if(sender == _rpid)
    return _r_readChannel;

  else
    qFatal("MergeSorter::_getReadChannel, sender is not a child process");
}

QVector<int> & MergeSorter::_getResultDestination(const pid_t & sender)
{
  if(sender == _lpid)
    return *_left_results;

  else if(sender == _rpid)
    return *_right_results;

  else
    qFatal("MergeSorter::_getResultDestination, sender is not a child process");
}

void MergeSorter::_mergeResults()
{

  _numbers->clear();

  while ( _left_results->size() > 0 && _right_results->size() > 0 )
    {
      if ( _left_results->front() < _right_results->front() )
	{
	  _numbers->push_back( _left_results->front() );

	  _left_results->erase( _left_results->begin() );
	}

      else if ( _left_results->front() == _right_results->front() )
	{
	  _numbers->push_back( _left_results->front() );
	  _numbers->push_back( _right_results->front() );

	  _left_results->erase( _left_results->begin() );
	  _right_results->erase( _right_results->begin() );
	}

      else
	{
	  _numbers->push_back( _right_results->front() );

	  _right_results->erase( _right_results->begin() );
	}
    }

  //éléments restant du vecteur le plus long

  for (QVector<int>::iterator it = _left_results->begin();
       it != _left_results->end();
       ++it)
    {
      _numbers->push_back(*it);
    }


  for (QVector<int>::iterator it = _right_results->begin();
       it != _right_results->end();
       ++it)
    {
      _numbers->push_back(*it);
    }

}
