//
//  main.cpp
//  zsLib.Eventing.Compiler.Tool
//
//  Created by Robin Raymond on 2016-11-21.
//  Copyright © 2016 Robin Raymond. All rights reserved.
//

#include <zsLib/eventing/tool/ICommandLine.h>
#include <zsLib/eventing/tool/OutputStream.h>

#include <iostream>
#include <signal.h>

namespace zsLib { namespace eventing { namespace tool { ZS_DECLARE_SUBSYSTEM(zslib_eventing_tool) } } }

using namespace zsLib::eventing::tool;

void my_handler(int s) {
  ICommandLine::interrupt();
  exit(1);
}

int main(int argc, char * const argv[])
{
  struct sigaction sigIntHandler;
  
  sigIntHandler.sa_handler = my_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;
  
  sigaction(SIGINT, &sigIntHandler, NULL);

  ICommandLine::StringList arguments;
  
  if (argc > 0) {
    argv = &(argv[1]);
    --argc;
  }
  
  output().installStdOutput();
  output().installDebugger();
  
  arguments = ICommandLine::toList(argc, argv);
  return ICommandLine::performDefaultHandling(arguments);
}
