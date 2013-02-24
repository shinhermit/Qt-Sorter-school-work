#include <cstdlib>
#include <iostream>

#include <QApplication>
#include <QString>

#include "Controler.hpp"
#include "MergeSorter.hpp"
#include "CommandLineParser.hpp"
#include "SortUtility.hpp"

int main(int argc, char ** argv)
{
  CommandLineParser parser(argc, argv);
  std::map<std::string, std::string> options;
  std::map<std::string, std::string>::iterator it;

  //Parsing command line: determines which program to run (Control or Sorter)
  bool controlProg = true;

  parser.parseInto(options);

  it = options.find("-p");

  if( it != options.end() )
    {
      if(it->second == "sorter")
	controlProg = false;
    }

  //initialising QApp (happens in any case)
  QApplication app(argc, argv);

  //Excution of Control program
  if(controlProg)
    {
      Controler * controler = new Controler(QString::fromStdString(options["-0"]));

      controler->show();
    }

  //Excution of Sorter program
  else
    {
      MergeSorter * sorter = new MergeSorter( QString::fromStdString(options["-0"]),
					      ::atoi(options["--control"].c_str()),
					      ::atoi(options["--write-channel"].c_str()),
					      ::atoi(options["--read-channel"].c_str()) );

      sorter->show();
    }

  return app.exec();
}
