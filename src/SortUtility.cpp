#include "SortUtility.hpp"

std::string SortUtility::itos(const int & i)
{
  std::ostringstream oss;

  oss << i;

  return oss.str();
}

QString SortUtility::SigCodeToQString(const SortTypes::SigCode & sigCode)
{
  QString stringSig("");

  switch(sigCode)
    {
    case SortTypes::STEP_MODE_DISABLED:
      stringSig = "STEP_MODE_DISABLED";
      break;

    case SortTypes::STEP_MODE_ENABLED:
      stringSig = "STEP_MODE_ENABLED";
      break;

    case SortTypes::SORT_PAUSED:
      stringSig = "SORT_PAUSED";
      break;

    case SortTypes::SORT_RESUMED:
      stringSig = "SORT_RESUMED";
      break;

    case SortTypes::SORT_REQUESTED:
      stringSig = "SORT_REQUESTED";
      break;

    case SortTypes::DATA_REQUESTED:
      stringSig = "DATA_REQUESTED";
      break;

    case SortTypes::DATA_SENT:
      stringSig = "DATA_SENT";
      break;

    case SortTypes::DATA_RECEIVED:
      stringSig = "DATA_RECEIVED";
      break;

    case SortTypes::RESULTS_AVAILABLE:
      stringSig = "RESULTS_AVAILABLE";
      break;

    case SortTypes::RESULTS_REQUESTED:
      stringSig = "RESULTS_REQUESTED";
      break;

    case SortTypes::RESULTS_SENT:
      stringSig = "RESULTS_SENT";
      break;

    case SortTypes::RESULTS_RECEIVED:
      stringSig = "RESULTS_RECEIVED";
      break;

    default:
      stringSig = QString("unknown signal (code ")+QString::number(sigCode)+")";
    }

  return stringSig;
}

SortTypes::SigCode SortUtility::IntToSigCode(const int & intVal)
{
  SortTypes::SigCode sigCode;

  switch(intVal)
    {
    case SortTypes::STEP_MODE_DISABLED:
      sigCode = SortTypes::STEP_MODE_DISABLED;
      break;

    case SortTypes::STEP_MODE_ENABLED:
      sigCode = SortTypes::STEP_MODE_ENABLED;
      break;

    case SortTypes::SORT_PAUSED:
      sigCode = SortTypes::SORT_PAUSED;
      break;

    case SortTypes::SORT_RESUMED:
      sigCode = SortTypes::SORT_RESUMED;
      break;

    case SortTypes::SORT_REQUESTED:
      sigCode = SortTypes::SORT_REQUESTED;
      break;

    case SortTypes::DATA_REQUESTED:
      sigCode = SortTypes::DATA_REQUESTED;
      break;

    case SortTypes::DATA_SENT:
      sigCode = SortTypes::DATA_SENT;
      break;

    case SortTypes::DATA_RECEIVED:
      sigCode = SortTypes::DATA_RECEIVED;
      break;

    case SortTypes::RESULTS_AVAILABLE:
      sigCode = SortTypes::RESULTS_AVAILABLE;
      break;

    case SortTypes::RESULTS_REQUESTED:
      sigCode = SortTypes::RESULTS_REQUESTED;
      break;

    case SortTypes::RESULTS_SENT:
      sigCode = SortTypes::RESULTS_SENT;
      break;

    case SortTypes::RESULTS_RECEIVED:
      sigCode = SortTypes::RESULTS_RECEIVED;
      break;

    default:
      sigCode = SortTypes::SIGNAL_CONVERSION_ERROR;
    }

  return sigCode;  
}
