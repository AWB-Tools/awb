/*****************************************************************************
 *
 *Copyright (C) 2004-2006 Intel Corporation
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
 * @author Ramon Matas Navarro
 * @brief Clock edge and phase definitions
 */

#ifndef _PHASE_
#define _PHASE_

#include <iostream>

using namespace std;

typedef enum clk_edge
{
    HIGH = 0,
    LOW,
    NUM_CLK_EDGES
} CLK_EDGE;

static const char* edgesName[NUM_CLK_EDGES] = {"H", "L"};

// Static array used to map a phase to a certain skew
static UINT32 clkEdges2skew[NUM_CLK_EDGES] = {0, 50};


// -------------------------------------------------------------------------
//
// Class phase

typedef struct phase
{
    UINT64 cycle;
    CLK_EDGE edge;
    
    phase() { }   
    phase(UINT64 _cycle, CLK_EDGE _edge) { cycle = _cycle; edge = _edge; }
    phase(UINT64 phases)
    {
        cycle = phases/2;
        if(phases%2==1) edge = LOW;
        else edge = HIGH;
    }
    
    UINT64 getPhaseNum() const
    {
        return (cycle*2 + edge);
    }

    UINT64 getCycle() const
    {
        return cycle;
    }

    // Operator overloading
    
    friend ostream& operator<< (ostream& os, const phase& phase)
    {
        os << phase.cycle << ':' << edgesName[phase.edge];
        return os;        
    }

    phase& operator=(phase &ph)
    {
        this->cycle = ph.cycle;
        this->edge = ph.edge;
        
        return *this;
    }
    
    bool operator== (const phase ph) const
    {
        return (this->cycle == ph.cycle) && (this->edge == ph.edge);
    }

    bool operator< (const phase ph) const
    {
        return ((this->cycle*2 + this->edge) < (ph.cycle*2 + ph.edge));
    }

    bool operator> (const phase ph) const
    {
        return ((this->cycle*2 + this->edge) > (ph.cycle*2 + ph.edge));
    }    

    bool operator<= (const phase ph) const
    {
        return ((this->cycle*2 + this->edge) <= (ph.cycle*2 + ph.edge));
    }    

    bool operator>= (const phase ph) const
    {
        return ((this->cycle*2 + this->edge) >= (ph.cycle*2 + ph.edge));
    }
        
    phase operator+(const phase &ph) const
    {
        UINT64 phases = this->getPhaseNum() + ph.getPhaseNum();
        phase temp(phases);
                
        return temp;
    }
    
    // + n phases
    phase operator+(const UINT64 n) const
    {
        UINT64 phases = this->getPhaseNum() + n;
        phase temp(phases);
        
        return temp;
    }

    phase operator-(const phase &ph) const
    {
        UINT64 phases = this->getPhaseNum() - ph.getPhaseNum();
        phase temp(phases);
        
        return temp;
    }    
    
    // - n phases
    phase operator-(const UINT64 n) const
    {
        UINT64 phases = this->getPhaseNum() - n;
        phase temp(phases);
        
        return temp;
    }
    
    // + one phase
    phase& operator++()
    {
        if(this->edge == LOW)
        {
            this->edge = HIGH;
            ++this->cycle;
        }
        else
        {
            this->edge = LOW;
        }
        
        return *this;
    }

    phase operator++(int)
    {
        phase temp(this->cycle, this->edge);
        
        if(this->edge == LOW)
        {
            this->edge = HIGH;
            ++this->cycle;
        }
        else
        {
            this->edge = LOW;
        }
        
        return temp;
    }
    
} PHASE;


#endif /* _PHASE_ */
