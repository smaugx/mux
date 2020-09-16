#pragma once


namespace mux {

class RunEntity {
public:
    RunEntity(const RunEntity&)              = delete;
    RunEntity& operator=(const RunEntity&)   = delete;
    RunEntity(RunEntity&&)                   = delete;
    RunEntity& operator=(RunEntity&&)        = delete;

    RunEntity()                               = default;
    virtual ~RunEntity()                      = default;

public:
    virtual bool Start()                      = 0;
    virtual bool Stop()                       = 0;
};



} // end namespace mux
