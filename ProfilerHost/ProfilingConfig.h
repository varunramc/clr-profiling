#pragma once
class ProfilingConfig
{
    std::wstring namespaceToProfile;
    std::wstring profilerCoreDllPath;

    ProfilingConfig();
    ~ProfilingConfig();

public:

    static ProfilingConfig& Get()
    {
        static ProfilingConfig instance;
        return instance;
    }

    const std::wstring& GetNamespaceToProfile() const
    {
        return this->namespaceToProfile;
    }

    const std::wstring& GetProfilerCoreDllPath() const
    {
        return this->profilerCoreDllPath;
    }

    void SetNamespaceToProfile(const std::wstring &namespaceToProfile)
    {
        this->namespaceToProfile = namespaceToProfile;
    }

    void SetProfilerCoreDllPath(const std::wstring &profilerCoreDllPath)
    {
        this->profilerCoreDllPath = profilerCoreDllPath;
    }
};

