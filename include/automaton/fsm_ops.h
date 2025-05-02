#ifndef ATOM_FSM_OPS_H
#define ATOM_FSM_OPS_H

#include "include/automaton/fsm.h"

namespace atom::automaton {

FSM Combine(const FSM& firhighPrioritystFSM, const FSM& lowPriorityFSM);

} //! namespace atom::automaton

#endif //! ATOM_FSM_OPS_H
