#ifndef _SortTypes
#define _SortTypes

struct SortTypes
{
  typedef int fd_t;

  typedef unsigned int seconds;

  enum SigCode
    {
      SIGNAL_CONVERSION_ERROR,
      STEP_MODE_DISABLED, STEP_MODE_ENABLED,
      SORT_PAUSED, SORT_RESUMED,
      SORT_REQUESTED,
      DATA_REQUESTED, DATA_SENT, DATA_RECEIVED, 
      RESULTS_AVAILABLE, RESULTS_REQUESTED, RESULTS_SENT, RESULTS_RECEIVED
    };
};

#endif
