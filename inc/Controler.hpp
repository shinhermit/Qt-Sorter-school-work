#ifndef _Controler
#define _Controler

#include <cstdlib>

#include <QWidget>
#include <QVector>
#include <QString>
#include <QDebug>
#include <QSocketNotifier>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/socket.h>

#include "SortUtility.hpp"
#include "ui_ControlView.h"
#include "MergeSorter.hpp"

class Controler : public QWidget
{
  Q_OBJECT

  public:
  Controler(const QString & execName);
  ~Controler();

  //Unix signal handlers
  static void sighandler(int signum, siginfo_t * info, void * secret);

private:
  Ui::ControlView * _ui;
  QString * _execName;
  QVector<int> * _numbers;

  //radom numbers generation requirements
  static bool _seeded;

  //Unix signals requirements
  struct sigaction _sigact;

  //Unix signal - Qt handling bridge
  static int _sigFd[2];
  QSocketNotifier * _sigNotifier;

  //signals information
  pid_t _sigEmitter;
  SortTypes::SigCode _sigCode;

  //Child process informations
  pid_t _sorter_pid;
  int _writeChannel;
  int _readChannel;

private slots:
  void _sigSlot();
  void _continueSlot();
  void _drawSlot();
  void _stepModeSlot();
  void _waitDurationSlot();
  void _pauseSlot();
  void _sortSlot();
  void _resetSlot();

private:
  void _setSigaction();
  void _updateStatus(const QString & status);

  void _sendData();

  void _createSorter();

  void _tell(const pid_t & recipient, const SortTypes::SigCode & sigCode);

  void _fetchResult();

  void _displaySortedList();
};

#endif
