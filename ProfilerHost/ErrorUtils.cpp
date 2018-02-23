#include "stdafx.h"
#include "ErrorUtils.h"
#include "Globals.h"
#include "StringUtils.h"

void LogComError(const _com_error& comError, const std::string& context)
{
    auto description = comError.Description();
    auto errorMessage = comError.ErrorMessage();
    auto err = comError.Error();

    std::stringstream msg;
    msg << "Encountered COM error while " << context << ". HRESULT: 0x" << std::hex << err;
    
    if (description.length())
    {
        msg << ", Description: " << description;
    }

    msg << ", HRESULT description: " << ToString(errorMessage);

    Logger->error(msg.str());
}
