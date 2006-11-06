/*****************************************************************************
 * dependency.cpp - tracks dependencies between objects (usually instructions)
 *
 *Copyright (C) 2003-2006 Intel Corporation
 *
 *This program is free software; you can redistribute it and/or
 *modify it under the terms of the GNU General Public License
 *as published by the Free Software Foundation; either version 2
 *of the License, or (at your option) any later version.
 *
 *This program is distributed in the hope that it will be useful,
 *but WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *GNU General Public License for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with this program; if not, write to the Free Software
 *Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/**
 * @file
 * @author Eric Borch
 * @brief Generic dependency tracking between instructions.
 */

// note, don't include anything to prevent building dependency info on
// registers that don't do anything for a particular implementation.
// For example, register 0 in IPF is always zero.  Don't need to build
// dependency for it.

// ASIM core
#include "asim/provides/dependency_objects.h"
#include "asim/atomic.h"

UID_GEN64 OUTPUT_DEP_CLASS::uniqueId = 0;
UID_GEN64 INPUT_DEP_CLASS::uniqueId = 0;

ASIM_MM_DEFINE(INPUT_DEP_CLASS, MAX_INPUT_DEP);
ASIM_MM_DEFINE(OUTPUT_DEP_CLASS, MAX_OUTPUT_DEP);

void
INPUT_DEP_CLASS::BuildRAWDependencyGraph(
    OUTPUT_DEP producer, 
    UINT64 cycle)
/*
 * This function builds the dependency graph for 
 * this single dependency object.
 */
{
    // if producer for num points to null (pm is starting up it's ready NOW
    if (producer == NULL)
    {
        this->SetCycleReadyForIssue(cycle);
    }
    
    // if producer has already produced it's value, go ahead and point consumers to
    // producer, but DON'T hook up consumers, since that's really used to notify
    // all consumers of a producer when the producer issues.  Also, set ready to
    // issue NOW
    // basically, we need to know if the producer is past its trap point.  This
    // isn't quite accurate right now.  FIX! Eric
    else if (producer->GetCycleValueProduced() <= cycle)
    {
        this->SetProducer(producer);
        this->SetCycleReadyForIssue(cycle);
    }
    // link this instr in the dependency chain after the lastInflightWriter
    // and link the rest of the chain in after it.  Must do this, even if
    // the instruction has already issued because it might get poisoned, in 
    // which case it'll need the dependency info to reissue at a later time.
    else 
    {           
        // we used to hook the consumers up in REVERSE order, just to save the
        // walk down the list every time you want to insert another consumer.
        // However, having them in order makes it a bit easier to find the first
        // consumer (which makes replay a bit easier in an in-order machine).
        // this->SetNextConsumer(producer->GetConsumer());
        // producer->SetConsumer(this);

        // get the first consumer of the producer
        INPUT_DEP consumer = producer->GetConsumer();

        // if a consumer doesn't exist, this is the first consumer, so hook it
        // up to the producer
        if (consumer == NULL)
        {
            producer->SetConsumer(this);
        }
        else // there IS a consumer
        {
            // walk down the list of consumers until we reach the end.
            while (consumer->GetNextConsumer())
            {
                consumer = consumer->GetNextConsumer();
            }
            // end of the consumers, so hook this consumer in at the end of the
            // list.
            this->SetPrevConsumer(consumer);
            consumer->SetNextConsumer(this);
        }

        // set nextConsumer to NULL, since we're inserting at the end of the
        // list.  nextConsumer should be NULL anyway, but this is just to be
        // safe.
        this->SetNextConsumer(NULL);

        // finally, set the producer of this consumer
        this->SetProducer(producer);
        
        // if the instruction producer points to has
        // issued, this will be data ready when the
        // producer says it's ready 
        // TO DO: this maybe should be checking
        // getcycledependentscanissue depending on whether instructions
        // that enter the IQ are data ready even if the producer poisoned
        // and have returned, waiting to issue - Eric
        if (producer->GetCycleIssued() != UINT64_MAX) 
        {
            this->SetCycleReadyForIssue(producer->GetCycleDependentsCanIssue());
        }
    }
}
/**
 * @brief This routine cleans up/removes all of the dependency links that
 * represent the dependency code.  It's called on the input_dep object, to allow
 * the output dependency object to use the dependency code even after the
 * producer has retired.  This should only be called when an input dependency
 * retires!
 */
void
INPUT_DEP_CLASS::RemoveSingleDependencyLink()
{
    OUTPUT_DEP prod;
    // check for any links and remove
    INPUT_DEP next = this->GetNextConsumer();
    INPUT_DEP prev = this->GetPrevConsumer();
    // get the producer of this input dependency
    prod = this->GetProducer();

    // at some point, we MUST null out the pointer to the instruction, so
    // that the instruction can null out pointer to this dependency object.
    // Since we no longer need this dependency object (we are about to remove it
    // from the list), might as well do it here.
    if (this->GetInst())
    {
        this->NullInst();
    }
    //if there is a producer and it thinks we are the head of its
    //chain of dependencies then update it
    if(prod && prod->GetConsumer() == this)
    {
        //I am pretty sure that if we are the consumer that the
        //producer points to then we should not have a prev consumer
        ASSERTX(!prev);
        prod->SetConsumer(next);
    }

    // remove it from the list of producers
    if (next)
    {
        next->SetPrevConsumer(prev);
    }
    
    if (prev)
    {
        prev->SetNextConsumer(next);
    }

    this->SetNextConsumer(NULL);
    this->SetPrevConsumer(NULL);


    // remove pointer back to producer of this dependency object
    this->SetProducer(NULL);

}
/**
 * @brief This routine cleans up/removes all of the dependency links that
 * represent the dependency code.  It's called on the input_dep object, to allow
 * the output dependency object to use the dependency code even after the
 * producer has retired.  This should only be called when an input dependency
 * retires!
 */
void 
INPUT_DEP_CLASS::RemoveDependencyLinks()
{
    OUTPUT_DEP prod;

    // get the producer of this input dependency
    prod = this->GetProducer();
    
    //The old form of input dependency links does not use the prev
    //consumer field so just set it to null
    this->SetPrevConsumer(NULL);

    // remove pointer back to producer of this dependency object
    this->SetProducer(NULL);

    // FIX: This routine HAS to be more consistent.  If the input dependency is
    // from a killed inst, we just remove this input dep from the chain.
    // However, if it's retired, we remove this input dep AND all others.
    // Probably should only remove this particular dependency for both cases.
    // Eric

    if (this->GetInst() && this->GetInst()->IsKilled() && prod)
    {
        INPUT_DEP consumer = prod->GetConsumer();
        INPUT_DEP youngestConsumer = NULL;

        while (consumer != NULL)
        {
            if (consumer == this)
            {
                break;
            }
            youngestConsumer = consumer;
            consumer = consumer->GetNextConsumer();
        }

        // it's possible if the producer has retired, that the consumer isn't in
        // it's list of consumers.  So, check to see if consumer is null before
        // we try to clean up the list.
        if (consumer != NULL)
        {
            if (youngestConsumer == NULL)
            {   // consumer was the first consumer in the list
                ASSERTX(consumer == prod->GetConsumer());
                
                prod->SetConsumer(consumer->GetNextConsumer());
                consumer->SetNextConsumer(NULL);
            }
            else
            {
                youngestConsumer->SetNextConsumer(consumer->GetNextConsumer());
                consumer->SetNextConsumer(NULL);
            }
        }
    }
    else if (prod)
    {
        // if there is a producer, remove the links from the producer to all of
        // it's consumers
        prod->RemoveDependencyLinks();
    }
    
    // by now, all of these pointers should be deleted.
    ASSERTX(this->GetProducer() == NULL);

    // is this comment still true?
    // this actually doesn't work.  The nextConsumer links are removed when the
    // producer is retired or killed.  If the consumer is down a bad path, but
    // the producer is on a good path, the consumer could get killed BEFORE the
    // producer retires, and the code will try and clean up the consumer links
    // first.  Thus, this assert will fail.  We could change it, but it works
    // for now.
    ASSERTX(this->GetNextConsumer() == NULL);
}

/**
 * When an input dependency (a src operand) retires, we need to clean up the
 * links that we used to signal when it was ready to issue.
 */
void
INPUT_DEP_CLASS::CleanUpLinks()
{

    // remove any left over dependency links
    this->RemoveDependencyLinks();

    // at some point, we MUST null out the pointer to the instruction, so
    // that the instruction can null out pointer to this dependency object.
    // Since the intruction that uses this src has committed, we have no
    // more use for it, so might as well do this here.
    this->NullInst();
    ASSERTX(this->GetProducer() == NULL);        
}

/**
 * This function builds the OUTPUT dependency graph for 
 * this single dependency object.
 */
void
OUTPUT_DEP_CLASS::BuildOutputDependencyGraph(
    OUTPUT_DEP prevProducer)
{
    this->SetPrevProducer(prevProducer);

    // go ahead and connect up producers
    if (prevProducer)
    {
        prevProducer->SetNextProducer(this);
    }
}

/**
 * This function sets potential issue time of an output dependency based upon
 * the WAW dependency graph for this single dependency object.
 */
void
OUTPUT_DEP_CLASS::IgnoreWAWDependencies(
    UINT64 cycle)
{
    this->SetCycleReadyForIssue(cycle);
}

/**
 * This function sets potential issue time of an output dependency based upon
 * the WAW dependency graph for this single dependency object.
 */
void
OUTPUT_DEP_CLASS::ObserveWAWDependencies(
    OUTPUT_DEP prevProducer, 
    UINT64 cycle)
{
    // if producer for num points to null (pm is starting up it's ready NOW
    if (prevProducer == NULL)
    {
        this->SetCycleReadyForIssue(cycle);
    }
    
    // go ahead set ready to issue time
    else 
    {
        // if the prevProducer has already produced its value, this output dependency
        // object can issue NOW.
        if (prevProducer->GetCycleValueProduced() <= cycle)
        {
            this->SetCycleReadyForIssue(cycle);
        }

        // if the prevProducer has issued, this output dependency object can
        // issue when the prevProducer says it can
        else if (prevProducer->GetCycleIssued() != UINT64_MAX) 
        {
            this->SetCycleReadyForIssue(prevProducer->GetCycleDependentsCanIssue());
        }
        // otherwise, the prevProducer is in flight, but hasn't issued.  When it
        // issues, it will inform this output dependency when it can issue.
    }
}

void 
OUTPUT_DEP_CLASS::SignalNextProducer(
    UINT64 cycle)
{
    // this is for WAW hazards and for dealing with predicated instructions
    // that may write the same register value.  If there is a consumer between
    // the 2 producers, then this may be irrelevant, but leaving it in here
    // shouldn't hurt anything.
    if (this->GetNextProducer())
    {
        // signal to the nextProducer that it can issue
        this->GetNextProducer()->SetCycleReadyForIssue(cycle);
    }

}

void 
OUTPUT_DEP_CLASS::UpdateNextProducerIssueTime()
{
    UINT64 cycleReady = this->GetCycleDependentsCanIssue();
    SignalNextProducer(cycleReady);
}

void 
OUTPUT_DEP_CLASS::SignalProducerToStopIssue()
{
    UINT64 cycleReady = UINT64_MAX;
    SignalNextProducer(cycleReady);
}

void 
OUTPUT_DEP_CLASS::SignalConsumers(
    UINT64 cycle)
/*
 */
{
    INPUT_DEP consumer;

    // use "consumer" as temporary pointer while moving down
    // dependency links
    consumer = this->GetConsumer();

    // if there is a consumer for this output, scan down dependency 
    // chain until there are none left
    while (consumer != NULL) 
    {
        // sanity check that the consumer thinks this producer is correct
//        ASSERTX(consumer->GetProducer() == this);

        // if you want to use cross cluster delays, pass the picker
        // the dependency object issued on to it's dependents.  The
        // dependents should store it and later use it to
        // calc. whether it can issue on a given picker.
        consumer->SetCycleReadyForIssue(cycle);
        consumer = consumer->GetNextConsumer();
    }
}

void 
OUTPUT_DEP_CLASS::UpdateConsumersIssueTime()
{
    UINT64 cycleReady = this->GetCycleDependentsCanIssue();
    SignalConsumers(cycleReady);
}

void 
OUTPUT_DEP_CLASS::SignalConsumersToStopIssue()
{
    UINT64 cycleReady = UINT64_MAX;
    SignalConsumers(cycleReady);
}

/*
 */
void 
OUTPUT_DEP_CLASS::RemoveDependencyLinks()
{
    INPUT_DEP consumer;
    INPUT_DEP youngestConsumer;

    // use "consumer" as temporary pointer while moving down and deleting
    // dependency links
    consumer = this->GetConsumer();

    // now that consumer points to the first consumer of destination in
    // chain, remove the Consumer pointer.
    this->SetConsumer(NULL);

    // if there is a consumer for this output, scan down dependency 
    // chain until there are none left
    while (consumer != NULL) 
    {
        // sanity check that the consumer thinks this producer is correct
        // CLEAN THIS CRAP UP!!!!! Specifically the way things are cleaned up
        // after killed!!!! Eric - the producer might be null if the consumer's
        // already been killed, and partially cleaned up
        ASSERTX(consumer->GetProducer() == this || consumer->GetProducer() == NULL);
                                
        // move down the chain to the next consumer.
        youngestConsumer = consumer;
        consumer = consumer->GetNextConsumer();

        // remove pointers between consumers.
        youngestConsumer->SetNextConsumer(NULL);
    }

    // this needs be removed, because an output dep. could be removed, but the
    // prior producer might be a miss that's still not resolved.
//    ASSERTX(prevProducer == NULL);

    // by now, all of these pointers should be deleted.
    ASSERTX(consumer == NULL);
}

/**
 */
void
OUTPUT_DEP_CLASS::CleanUpLinks()
{
    OUTPUT_DEP next = this->GetNextProducer();
    OUTPUT_DEP prev = this->GetPrevProducer();
    this->RemoveDependencyLinks();
    // at some point, we MUST null out the pointer to the instruction, so
    // that the instruction can null out pointer to this dependency object.
    // Since we no longer need this dependency object (we are about to remove it
    // from the list), might as well do it here.
    if (this->GetInst())
    {
//        this->GetInst()->NullDstDep(this);
        this->NullInst();
    }
    
    // remove it from the list of producers
    if (next)
    {
        next->SetPrevProducer(prev);
    }
    
    if (prev)
    {
        prev->SetNextProducer(next);
    }

    this->SetNextProducer(NULL);
    this->SetPrevProducer(NULL);
}
