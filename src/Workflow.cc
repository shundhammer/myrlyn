/*  ------------------------------------------------------
              __   _____  ____  _
              \ \ / / _ \|  _ \| | ____ _
               \ V / | | | |_) | |/ / _` |
                | || |_| |  __/|   < (_| |
                |_| \__\_\_|   |_|\_\__, |
                                    |___/
    ------------------------------------------------------

    Project:  YQPkg Package Selector
    Copyright (c) 2024 SUSE LLC
    License:  GPL V2 - See file LICENSE for details.

    Textdomain "qt-pkg"
 */


#include <QSet>

#include "Logger.h"
#include "Exception.h"
#include "Workflow.h"

#ifndef VERBOSE_WORKFLOW
#  define VERBOSE_WORKFLOW      0
#endif


Workflow::Workflow( const WorkflowStepList & steps )
{
    _steps = steps; // This takes ownership of the steps!

    if ( _steps.isEmpty() )
    {
        logError() << "Empty workflow!" << Qt::endl;
        return;
    }

    checkDuplicateIds();

    foreach ( WorkflowStep * step, _steps )
    {
        CHECK_PTR( step );
        step->setWorkflow( this );
    }
}


Workflow::~Workflow()
{
    qDeleteAll( _steps ); // Call 'delete' for each list item
}


void Workflow::checkDuplicateIds()
{
    QSet<QString> ids;
    QStringList duplicates;

    foreach ( WorkflowStep * step, _steps )
    {
        CHECK_PTR( step );

        if ( ids.contains( step->id() ) )
            duplicates << step->id();
        else
            ids << step->id();
    }

    if ( ! duplicates.isEmpty() )
    {
        logError() << "Duplicate workflow step IDs: " << duplicates << Qt::endl;
        THROW( Exception( "Duplicate worflow step IDs" ) );
    }
}


WorkflowStep *
Workflow::step( const QString & id ) const
{
    foreach ( WorkflowStep * step, _steps )
    {
        if ( step->id() == id )
            return step;
    }

    logWarning() << "No workflow step \"" << id << "\"" << Qt::endl;
    return 0;
}


void Workflow::next()
{
    if ( _currentStep && ! _currentStep->next().isEmpty() ) // Step has a nonstandard 'next'
    {
        activate( step( _currentStep->next() ) );
    }
    else  // No nonstandard next -> use the next step in the steps list
    {
        int index = _steps.indexOf( _currentStep );      // -1 if not found

        if ( index >= 0 && index + 1 < _steps.size() )  // in range?
            activate( _steps.at( index + 1 ) );
        else
            logError() << "No workflow step after step " << _currentStep->id()
                       << " (#" << index << ")"
                       << Qt::endl;
    }
}


void Workflow::back()
{
    if ( _history.isEmpty() )
    {
        logWarning() << "Workflow steps history is empty - can't go back any further" << Qt::endl;
        return;
    }

    activate( popHistory(),
              false ); // goingForward
}


void Workflow::gotoStep( const QString & id )
{
    activate( step( id ) );
}


void Workflow::activate( WorkflowStep * step, bool goingForward )
{
    if ( step )
    {
#if VERBOSE_WORKFLOW
        logDebug() << "Going " << QString( goingForward ? "forward" : "backward" )
                   << " to step " << step->id()
                   << Qt::endl;
#endif

        if ( _currentStep )
            _currentStep->deactivate( goingForward );

        if ( goingForward && _currentStep )
        {
            pushHistory( _currentStep );
        }

        _currentStep = step;
        step->activate( goingForward );
    }
}


void Workflow::start()
{
#if VERBOSE_WORKFLOW
        logDebug() << "Starting the workflow" << Qt::endl;
#endif

    restart();
}


void Workflow::restart()
{
    _history.clear();
    _currentStep = _steps.isEmpty() ? 0 : _steps.first();

    if ( _currentStep )
        _currentStep->activate( true ); // goingForward
}


bool Workflow::atLastStep() const
{
    if ( _steps.isEmpty() )
        return true;

    CHECK_PTR( _currentStep );
    int index = _steps.indexOf( _currentStep );

    if ( index < 0 )
    {
        logError() << "Can't find current step in the steps list" << Qt::endl;
        return true;
    }

    return index == _steps.size() - 1;
}


void Workflow::pushHistory( WorkflowStep * step )
{
    if ( step )
    {
        if ( ! step->includeInHistory() )
        {
#if VERBOSE_WORKFLOW
            logDebug() << "Step " << step->id() << " is excluded from history" << Qt::endl;
#endif
        }
        else
        {
#if VERBOSE_WORKFLOW
            logDebug() << "Saving step " << step->id() << " to history" << Qt::endl;
#endif
        _history << step;
        }
    }
    else
        logError() << "Refusing to push null pointer to history" << Qt::endl;
}


WorkflowStep *
Workflow::popHistory()
{
    if ( _history.isEmpty() )
    {
        logError() << "History is empty" << Qt::endl;
        return 0;
    }

    WorkflowStep * step = _history.takeLast();
    CHECK_PTR( step );

#if VERBOSE_WORKFLOW
    logDebug() << "Taking step " << step->id() << " from history" << Qt::endl;
#endif

    return step;
}

