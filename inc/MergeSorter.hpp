#ifndef _MergeSorter
#define _MergeSorter

#include <QWidget>
#include <QVector>
#include <QSocketNotifier>
#include <QString>
#include <QDebug>
#include <QTimer>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/socket.h>

#include "ui_SorterView.h"
#include "SortTypes.hpp"
#include "SortUtility.hpp"

class MergeSorter : public QWidget
{
  Q_OBJECT

  public:
  MergeSorter(const QString & execName,
	      const pid_t & ppid,
	      const SortTypes::fd_t & writeChannel,
	      const SortTypes::fd_t & readChannel);

  ~MergeSorter();

  static void sighandler(int signum, siginfo_t * info, void * secret);

private:
  Ui::SorterView * _ui;
  QString * _execName;
  QVector<int> * _numbers;

  //Unix signal requirements
  struct sigaction _sigact;

  //Unix signal - Qt handling bridge
  static int _sigFd[2];
  QSocketNotifier * _sigNotifier;

  //Signal information
  pid_t _sigEmitter;
  SortTypes::SigCode _sigCode;

  //Communication with Parent process
  pid_t _ppid; /*!< parent pid*/

  int _p_writeChannel; /*!< write to parent */
  int _p_readChannel; /*!< read from parent */

  //Children processes requirements
  QVector<int> * _left_results;
  QVector<int> * _right_results;
  short _partsReceived;

  pid_t _lpid; /*!< left (child) pid */
  pid_t _rpid; /*!< right (child) pid */

  int _l_writeChannel; /*!< write to left child */
  int _l_readChannel; /*!< read from left child */

  int _r_writeChannel; /*!< write to right child */
  int _r_readChannel; /*!< read from right child */


private slots:
  void _sigSlot();
  void _sortSlot();
  void _validationSlot();

private:
  void _setSigAction();

  void _updateStatus(const QString & status);
  void _updateDisplayedList(QLineEdit & listUI, const QVector<int> & numbers, const int & begin, const int & end);
  void _updateDisplayedList(QLineEdit & listUI, const QVector<int> & numbers);

  void _fetchData(const int & readChannel, QVector<int> & destination);
  void _sendData(const int & writeChannel, QVector<int> & dataSource, const int & begin, const int & end);
  void _sendData(const int & writeChannel, QVector<int> & dataSource);

  void _tell(const pid_t & recipient, const SortTypes::SigCode & sigCode);

  void _dispatchWork();
  void _createSorter(pid_t & getCreatedPid, int & getWriteChannel, int & getReadChannel, const int & dataBegin, const int & dataEnd);

  int & _getReadChannel(const pid_t & sender);
  QVector<int> & _getResultDestination(const pid_t & sender);

  void _mergeResults();
};

#endif
