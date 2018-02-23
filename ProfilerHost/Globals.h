#pragma once

#include "stdafx.h"
#define Logger Globals::Instance().GetLogger()

class Globals
{
private:
    std::shared_ptr<spdlog::logger> logger;

public:
    Globals()
    {
        this->logger = spdlog::stdout_color_mt("console");
    }

    static Globals& Instance()
    {
        static Globals instance;
        return instance;
    }

    spdlog::logger* GetLogger() const
    {
        return this->logger.get();
    }
};
