#include "Controler.hpp"

bool Controler::_seeded = false;
int Controler::_sigFd[2] = {-1, -1};

Controler::Controler(const QString & execName)
  :_ui( new Ui::ControlView() ),
   _execName( new QString(execName) ),
   _numbers( new QVector<int>() )
{
  _ui->setupUi(this);

  //Unix signals
  _setSigaction();

  //Unix handler to Qt handling
  if ( ::socketpair(AF_UNIX, SOCK_STREAM, 0, _sigFd) == -1 )
    qFatal("::socketpair, in Controler::Controler.");

  _sigNotifier = new QSocketNotifier(_sigFd[0], QSocketNotifier::Read, this);
  connect(_sigNotifier, SIGNAL(activated(int)), this, SLOT(_sigSlot()));

  //Initial draw
  _drawSlot();
}

Controler::~Controler()
{
  delete _ui;
  delete _execName;
  delete _numbers;
  delete _sigNotifier;
}

void Controler::sighandler(int signum, siginfo_t * info, void * secret)
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

void Controler::_sigSlot()
{
  _updateStatus("Signal notification received");

  _sigNotifier->setEnabled(false);

  if( ::read(_sigFd[0], &_sigEmitter, sizeof(pid_t)) == -1)
    qFatal("read (1), in Controler::sigSlot");

  if( ::read(_sigFd[0], &_sigCode, sizeof(SortTypes::SigCode)) == -1)
    qFatal("read (2), in Controler::sigSlot");

  switch(_sigCode)
    {
    case SortTypes::RESULTS_SENT:
      _fetchResult();
      _displaySortedList();
      _tell(_sorter_pid, SortTypes::RESULTS_RECEIVED);
      break;

    default:
      qFatal("Controler::_sigSlot, unexpected signal");
    }

  _sigNotifier->setEnabled(true);  

  qDebug() << "Received SigCode " << SortUtility::SigCodeToQString(_sigCode) << "from (" << _sigEmitter << ")" << "\n";
}

void Controler::_continueSlot()
{
  qDebug() << "Controler::_continueSlot, Continue Pressed";
}

void Controler::_drawSlot()
{
  int size, val;
  QString listString;

  if(!_seeded)
    {
      ::srand( ::time(NULL) );
      _seeded = true;
    }

  size = _ui->listSizeSpinBox->value();

  _numbers->clear();
  listString = "";

  for(int i = 0; i < size; ++i)
    {
      val = ::rand() % 100;
      _numbers->push_back(val);
      listString += QString::number(val);
      if(i < size-1)
	listString += ", ";
    }
  
  _ui->initialListLineEdit->setText( listString );

  _updateStatus("Random numbers drawn");

}

void Controler::_stepModeSlot()
{
  qDebug() << "Controler::_stepModeSlot, Step mode changed";
}

void Controler::_waitDurationSlot()
{
  qDebug() << "Controler::_waitDurationSlot, wait duration button pressed";
}

void Controler::_pauseSlot()
{
  qDebug() << "Controler::_pauseSlot, Pause button Pressed";
}

void Controler::_sortSlot()
{
  _createSorter();
}

void Controler::_resetSlot()
{
  qDebug() << "Controler::_resetSlot, Reset button Pressed";
}

void Controler::_updateStatus(const QString & status)
{
  _ui->statusPlainTextEdit->setPlainText(QString(". ")+status+"\n" + _ui->statusPlainTextEdit->toPlainText());
}

void Controler::_setSigaction()
{

  if( sigemptyset(&_sigact.sa_mask) == -1)
    qFatal("sigemptyset, in Controler::_setSigAction");

  _sigact.sa_flags = SA_SIGINFO|SA_RESTART;
  _sigact.sa_sigaction = sighandler;

  if( sigaction(SIGUSR1, &_sigact, NULL) == -1)
    qFatal("sigaction, in Controler::_setSigAction");

}

void Controler::_sendData()
{
  QVector<int> & numbers = *_numbers;

  int datalen = numbers.size();

  //data length
  if( ::write(_writeChannel, &datalen, sizeof(int)) == -1)
    qFatal("write (1), in Controler::_sendData");

  //actual data
  for(int i=0; i < numbers.size(); ++i)
    {

      if( ::write(_writeChannel, &numbers[i], sizeof(int)) == -1)
	qFatal("write (2), in Controler::_sendData");
    }

  _updateStatus("Data sent in pipe" );

}

void Controler::_createSorter()
{
  int dad_writes[2];
  int son_writes[2];
  pid_t control_pid;

  control_pid = getpid();

  if( pipe(dad_writes) == -1 || pipe(son_writes) == -1  )
    qFatal("pipe, in Controler::_createSorter");

  _writeChannel = dad_writes[1];
  _readChannel = son_writes[0];

  //make list of numbers available for Sorter
  _sendData();

  switch( _sorter_pid = fork() )
    {
    case -1:
      qFatal("fork, in Controler::_createSorter");

    case 0:

      ::close(dad_writes[1]);
      ::close(son_writes[0]);

      setpgid(_sorter_pid, control_pid);

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
	qFatal("execl, in Controler::_createSorter");

    default:
      ::close(dad_writes[0]);
      ::close(son_writes[1]);

    }

  qDebug() << "write Channnels --- Control (" << control_pid << "): " << dad_writes[1] << ", Sorter (" << _sorter_pid <<"): " << son_writes[1] << "\n";
  qDebug() << "read Channnels --- Control (" << control_pid << "): " << son_writes[0] << ", Sorter (" << _sorter_pid <<"): " << dad_writes[0] << "\n";

  _updateStatus("Sorter view created" );
}

void Controler::_tell(const pid_t & recipient, const SortTypes::SigCode & sigCode)
{
  union sigval message;

  message.sival_int = sigCode;

  if( ::sigqueue(recipient, SIGUSR1, message) == -1)
    qFatal("sigqueue, in Controler::_tell");

  _updateStatus(QString("Signal ")+SortUtility::SigCodeToQString(sigCode)+" sent" );
}

void Controler::_fetchResult()
{
  int datalen, data;

  //data length
  if( ::read(_readChannel, &datalen, sizeof(int)) == -1 )
      qFatal("read (1), in MergeSorter::_fetchResult");

  //actual data
  _numbers->clear();

  for(int i=0; i < datalen; ++i)
    {
      if( ::read(_readChannel, &data, sizeof(int)) == -1 )
	qFatal("read (2), in MergeSorter::_fetchResult");

      _numbers->push_back(data);
    }

  _updateStatus("Received sorted list" );
}

void Controler::_displaySortedList()
{
  QString listString = "";
  QVector<int> & numbers = *_numbers;

  int size = numbers.size();

  for(int i = 0; i < size; ++i)
    {
      listString += QString::number(numbers[i]);

      if(i < size-1)
	listString += ", ";
    }

  _ui->sortedListLineEdit->setText( listString );
}
