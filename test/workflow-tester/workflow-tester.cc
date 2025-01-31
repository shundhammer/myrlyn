/*
    Project:  Myrlyn Package Manager GUI
    Copyright (c) 2024-25 SUSE LLC
    License:  GPL V2 - See file LICENSE for details.

    Textdomain "qt-pkg"
 */


#include <unistd.h>     // sleep()
#include <QString>

#include "../../src/Workflow.h"
#include "../../src/Logger.h"
#include "../../src/Exception.h"


class TestWfStep: public WorkflowStep
{
public:
    TestWfStep( const QString & id,
                const QString & next = QString() )
        : WorkflowStep( id, next )
        {}

    void activate  ( bool goingForward ) override;
    void deactivate( bool goingForward ) override;
};


void TestWfStep::activate( bool goingForward )
{
    logDebug() << "  +++ " << _id << Qt::endl;
}


void TestWfStep::deactivate( bool goingForward )
{
    logDebug() << "--- " << _id << Qt::endl;
}


int main( int argc, char *argv[] )
{
    Logger logger( "/tmp/myrlyn-$USER", "workflow-tester.log" );

    WorkflowStepList workflowSteps;
    workflowSteps << new TestWfStep( "Init Phase" )
                  << new TestWfStep( "Page 101" )
                  << new TestWfStep( "Page 102", "Page 103" ) // next: Page 103
                  << new TestWfStep( "Rare Page 99" )      // Should almost never be seen
                  << new TestWfStep( "Page 103" )
                  << new TestWfStep( "Page 104" )
                  << new TestWfStep( "Page 199 (last)" );

    Workflow * workflow = new Workflow( workflowSteps );

    workflow->step( "Init Phase" )->setExcludeFromHistory();
    workflow->start();


    logDebug() << "Going through the workflow past the end" << Qt::endl;

    for ( int i=0; i < workflowSteps.size() + 2; i++ )
    {
        workflow->next();
    }

    sleep( 1 );
    logNewline();
    logDebug() << "Going back 3 steps" << Qt::endl;

    for ( int i=0; i < 3; i++ )
    {
        workflow->back();
    }

    sleep( 1 );
    logNewline();
    logDebug() << "Going to page 104 directly" << Qt::endl;

    workflow->gotoStep( "Page 104" );

    sleep( 1 );
    logNewline();
    logDebug() << "Restarting the workflow" << Qt::endl;

    workflow->restart();

    logDebug() << "The history should be empty now: "
               << QString( workflow->historyEmpty() ? "empty" : "not empty" )
               << Qt::endl;

    sleep( 1 );
    logNewline();
    logDebug() << "Trying to go back 2 times" << Qt::endl;
    workflow->back();
    workflow->back();

    logDebug() << "Current step: " << workflow->currentStep()->id() << Qt::endl;

    sleep( 1 );
    logNewline();
    logDebug() << "Going forward until the end" << Qt::endl;

    while ( ! workflow->atLastStep() )
    {
        logDebug() << "Next" << Qt::endl;

        workflow->next();
    }


    sleep( 1 );
    logNewline();
    logDebug() << "Going backward until the start of the history" << Qt::endl;

    while ( ! workflow->historyEmpty() )
    {
        logDebug() << "Back" << Qt::endl;

        workflow->back();
    }


    sleep( 1 );
    logNewline();
    logDebug() << "Going on a wild ride" << Qt::endl;

    workflow->gotoStep( "Rare Page 99" ); // the one that is usually skipped
    workflow->gotoStep( "Page 104" );
    workflow->gotoStep( "Page 102" );
    workflow->gotoStep( "Page 199" );
    workflow->gotoStep( "Page 101" );


    sleep( 1 );
    logNewline();
    logDebug() << "Going backward until the start of the history" << Qt::endl;

    while ( ! workflow->historyEmpty() )
    {
        workflow->back();
    }

    delete workflow;

    return 0;
}
