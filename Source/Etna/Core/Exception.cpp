#include "Exception.h"
#include <sstream>

Exception::Exception(int line, const char* file, const std::string& msg)
    :
    m_Line(line), m_File(file), m_Msg(msg)
{}

const char* Exception::what() const noexcept
{
    std::stringstream ss;
    ss  << "[File] " << m_File << std::endl
        << "[Line] " << m_Line << std::endl
        << "[Msg ] " << m_Msg;

    m_WhatBuffer = ss.str();
    return m_WhatBuffer.c_str();
}
