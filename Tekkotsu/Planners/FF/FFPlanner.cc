#include "Shared/RobotInfo.h"
#if !defined(PLATFORM_APERIOS) && !defined(TGT_IS_AIBO)
#include <stdio.h>
#include <unistd.h>

#include "Events/EventRouter.h"
#include "FFPlanner.h"

void FFPlanner::plan(const std::string &problemFileName, const std::string &_planFileName) {
  planFileName = _planFileName;
  launchFF(problemFileName);
  erouter->addTimer(this, 9999, 100, true);
}

void FFPlanner::launchFF(const std::string &problemFileName) {
  pid_t child_id = fork();
  if ( child_id == 0 ) {
    char* tekrootval = getenv("TEKKOTSU_ROOT");
    std::string const tekkotsuRoot = tekrootval==NULL ? "/usr/local/Tekkotsu" : std::string(tekrootval);
    std::string const ffScriptName = "invoke-ff";
    std::string const ffScriptPath = tekkotsuRoot + "/Planners/FF/bin/" + ffScriptName;
    std::string const PlanFile = "/tmp/tekkotsu-PlannerOutput.txt";
    execlp(ffScriptPath.c_str(), ffScriptName.c_str(), domainFileName.c_str(), problemFileName.c_str(), planFileName.c_str(), NULL);
    // If we get here, the execlp() failed
    std::cerr << "ERROR: failed to launch invoke-ff script from " << ffScriptPath << std::endl
	      << "Check that TEKKOTSU_ROOT is set properly." << std::endl;
    _exit(0);
  }
}

void FFPlanner::doEvent() {
  if ( event->getGeneratorID() == EventBase::timerEGID && event->getSourceID() == 9999) {
    FILE *in = fopen(planFileName.c_str(), "r");
    if ( in != NULL ) {
      erouter->removeTimer(this,9999);
      size_t BUFFSIZE = 32768;
      char buffer[BUFFSIZE];
      size_t res = fread(&buffer, 1, BUFFSIZE-1, in);
      if ( feof(in) == 0 ) {
        std::cout << "Error reading file " << planFileName << " in FFPlanner::doEvent()" << std::endl;
      } else {
        buffer[res] = 0;
        result = std::string(buffer);
      }
      //erouter->postEvent(EventBase(EventBase::aiEGID, (size_t)this, EventBase::statusETID));
    }
  }
}

#endif