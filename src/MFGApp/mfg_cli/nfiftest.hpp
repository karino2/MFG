// Copyright 2026 Kazuma Arino. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef _NOT_FANCY_INTELLISENSE_FRIENDLY_TEST_H_
#define _NOT_FANCY_INTELLISENSE_FRIENDLY_TEST_H_

#include <algorithm>
#include <cassert>
#include <cstring>
#include <functional>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <string>
#include <sstream>

#ifdef _MSC_VER
    // MSVC complains for switch-case of enum which is exaustive and no default case section.
    // But clang complains if you add default case in the same situation. We conform to clang.
    #pragma warning( disable: 4715 )
#endif

namespace nfiftest {
struct TestPair {
    const char* _name;
    std::function<void()> _func;
    TestPair(const char* name, void func()) : _name(name), _func(func) {}
};



enum class SectionState {
    NOT_RUN_YET,
    FIRST_RUN,
    INSIDE,
    WAIT_SUCCESS, // becomes success at next call. Need this state to leave when success.
    SUCCESS,
    FAIL
};
struct SectionEntry {
    const char* _name;
    SectionState _state;
    SectionEntry* _parent;
    bool _waitingChildExist = false;
    SectionEntry(const char* name, SectionEntry* parent) : _name(name), _state(SectionState::NOT_RUN_YET), _parent(parent) {}

    std::vector<SectionEntry> _children;
    std::vector<SectionEntry>::iterator findEntry(const char *name)
    {
        return std::find_if(_children.begin(), _children.end(), [name](SectionEntry& ent) { return strcmp(name, ent._name) == 0; });
    }

    bool isEnd() { return _state == SectionState::SUCCESS || _state == SectionState::FAIL; }

};

struct Context {
    bool _leavingByError = false;
    int _successCount = 0;
    int _failCount = 0;

    void addSuccess() { _successCount++; }
    void addFail() { _failCount++; }

    void enter() { 
        // clear
        _current->_waitingChildExist = false;
    }
    void leave() {
        if(_leavingByError) {
            _current->_state = SectionState::FAIL;
            _leavingByError = false;
            _current = _current->_parent;
            return;
        }

        if(!_current->_waitingChildExist) {
            if(_current->_children.empty()
                || _current->_children.back()._state == SectionState::SUCCESS
                || _current->_children.back()._state == SectionState::WAIT_SUCCESS) {
                _current->_state = SectionState::WAIT_SUCCESS;
                if(_current->_children.empty())
                    addSuccess();
            } else if(_current->_children.back()._state == SectionState::FAIL) {
                _current->_state = SectionState::FAIL;
            } else if (_current->_children.back()._state == SectionState::NOT_RUN_YET) {
                // from begging of this section to first child.
                addSuccess();
            }
        } else if(_current->_waitingChildExist && _current->_state == SectionState::FIRST_RUN) {
            // from beggining of this section to first child.
            addSuccess();
        }
        _current = _current->_parent;
    }
    SectionEntry *_current;
    std::vector<SectionEntry>::iterator findEntry(const char *name) { return _current->findEntry(name); }
    std::vector<SectionEntry>::const_iterator endEntry() const { return _current->_children.end(); }
    void resetRoot(SectionEntry *newRoot) {
        _current = newRoot;
        _leavingByError = false;
    }
};


#ifdef _NFIFTEST_SUBTEST_
extern Context g_context;
#else
Context g_context;
#endif

struct assert_fail_error : std::runtime_error {
    std::string _file;
    int _line;
    SectionEntry *_current;
    assert_fail_error(std::string file, int line, std::string msg) : std::runtime_error(msg), _file(file), _line(line) {
        g_context._leavingByError = true;
        _current = g_context._current;
    }
};

#define REQUIRE(expr) if(!(expr)) throw nfiftest::assert_fail_error(__FILE__, __LINE__, #expr)


// section guard
struct SG {
    SG() { g_context.enter(); }
    ~SG() { g_context.leave(); }    
};




inline bool SECTION(const char* secName) {
    auto ent = g_context.findEntry(secName);
    if(ent == g_context.endEntry())
    {// not found, first call.
        if(g_context._current->_children.empty()) {
            if(g_context._current->_state == SectionState::NOT_RUN_YET) {
                g_context._current->_waitingChildExist = true;
                return false;
            }
            // parent reach to first child.
            // This case, the test from parentBegin till here is one case.
            // So return false this time.
            SectionEntry ent {secName, g_context._current};
            g_context._current->_children.push_back(ent);
            // SG is not entered. so _current must keep parent's one.
            // g_context._current = &g_context._current->_children.back();
            return false;
        }
        // parent reach subsequence child.
        // in this case first parent block is already passed.
        // So just go to this section.
        if(g_context._current->_children.back().isEnd()) {
            SectionEntry ent {secName, g_context._current};
            ent._state = SectionState::FIRST_RUN;
            g_context._current->_children.push_back(ent);
            g_context._current = &g_context._current->_children.back();
            return true;
        }
        // prev seciton is not yet end. wait until finish.
        g_context._current->_waitingChildExist = true;
        return false;
    }
    switch(ent->_state)
    {
        case SectionState::NOT_RUN_YET:
            ent->_state = SectionState::FIRST_RUN;
            g_context._current = &(*ent);
            return true;
        case SectionState::FIRST_RUN:
            ent->_state = SectionState::INSIDE;
            g_context._current = &(*ent);
            return true;
        case SectionState::INSIDE:
            g_context._current = &(*ent);
            return true;
        case SectionState::WAIT_SUCCESS:
            // goto next sibling.
            ent->_state = SectionState::SUCCESS;
            return false;
        case SectionState::SUCCESS:
        case SectionState::FAIL:
            return false;
    }
}

inline void RunTests(std::vector<TestPair>& testCases)
{
    std::vector<std::string> errors;
    g_context._successCount = 0;
    g_context._failCount = 0;
    
    for(const auto& test: testCases) {
        SectionEntry root {test._name, nullptr};
        root._state = SectionState::FIRST_RUN;
        while(!root.isEnd() && root._state != SectionState::WAIT_SUCCESS) {
            g_context.resetRoot(&root);
            SG g;
            try {
                test._func();
                // success current run
                assert(&root == g_context._current);
                switch(g_context._current->_state)
                {
                    case SectionState::FIRST_RUN:
                        if(g_context._current->_waitingChildExist) {
                            // pass root to first section
                            g_context.addSuccess();
                            g_context._current->_state = SectionState::INSIDE;
                        }// other case will handle correctly by root SG.
                        continue;
                    case SectionState::INSIDE:
                        break;
                    case SectionState::NOT_RUN_YET:
                    case SectionState::WAIT_SUCCESS:
                    case SectionState::SUCCESS:
                    case SectionState::FAIL:
                        assert(false); // never reached here.
                        break;
                }
            }catch(assert_fail_error &e) {
                std::stringstream ss;

                ss << "TEST FAIL: " << root._name << std::endl;
                ss << "   SECTION: " << e._current->_name << std::endl;
                ss << e._file << ":" << e._line << " REQUIRE FAIL  " << e.what() << std::endl;
                errors.push_back(ss.str());
                e._current->_state = SectionState::FAIL;
                g_context.addFail();
                continue;
            }catch(std::runtime_error& r) {
                std::stringstream ss;

                ss << "TEST FAIL: " << root._name << std::endl;
                ss << "   with unknown error: " << r.what() << std::endl;
                errors.push_back(ss.str());
                g_context.addFail();
                continue;
            }
        }
    }
    if(g_context._failCount == 0) {
        std::cout << "All " << g_context._successCount << " test passed" << std::endl;
    } else {
        std::cout << "=========== Fail " << g_context._failCount << " section ============= " << std::endl;
        std::cout << std::endl;
        for(const auto& msg : errors) {
            std::cout << msg << std::endl;
        }
    }
}


} // namespace

#endif