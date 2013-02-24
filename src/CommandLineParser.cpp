#include "CommandLineParser.hpp"

CommandLineParser::CommandLineParser(const int & argc, char** argv):_argc(argc), _argv(argv){}

void CommandLineParser::parseInto(std::map<std::string, std::string> & args)
{

  for(int i=0; i < _argc; ++i)
    {

      if(_argv[i][0] == '-')
	{

	  std::string option(_argv[i]);

	  args.insert( std::pair<std::string, std::string>( option, std::string("") ) );

	  if(i+1 < _argc)
	    {
	      if(_argv[i+1][0] != '-')
		{
		  args[option] = std::string(_argv[i+1]);
		  ++i;
		}
	    }

	}

      else
	{
	  args.insert( std::pair<std::string, std::string>( std::string("-")+SortUtility::itos(i), std::string(_argv[i]) ) );
	}
    }
}

std::map<std::string, std::string> CommandLineParser::parse()
{
  std::map<std::string, std::string> parsed;

  parseInto(parsed);

  return parsed;
}
