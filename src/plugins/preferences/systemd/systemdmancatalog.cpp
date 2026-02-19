#include "systemdmancatalog.h"

namespace
{

using preferences::MandatoryGroup;
using preferences::MandatoryProfile;
using preferences::MandatoryScaffoldEntry;
using preferences::SystemdUnitType;

QStringList mergeKeyLists(const QList<QStringList> &lists)
{
    QSet<QString> uniqueKeys;
    for (const auto &list : lists)
    {
        for (const auto &key : list)
        {
            uniqueKeys.insert(key);
        }
    }

    QStringList result = uniqueKeys.values();
    result.sort(Qt::CaseInsensitive);
    return result;
}

bool equalsInsensitive(const QString &lhs, const QString &rhs)
{
    return QString::compare(lhs.trimmed(), rhs, Qt::CaseInsensitive) == 0;
}

const QStringList &unitSectionKeys()
{
    static const QStringList keys{
        "After",
        "AllowIsolate",
        "AssertACPower",
        "AssertArchitecture",
        "AssertCPUFeature",
        "AssertCPUPressure",
        "AssertCPUs",
        "AssertCapability",
        "AssertControlGroupController",
        "AssertCredential",
        "AssertDirectoryNotEmpty",
        "AssertEnvironment",
        "AssertFileIsExecutable",
        "AssertFileNotEmpty",
        "AssertFirstBoot",
        "AssertGroup",
        "AssertHost",
        "AssertIOPressure",
        "AssertKernelCommandLine",
        "AssertKernelModuleLoaded",
        "AssertKernelVersion",
        "AssertMemory",
        "AssertMemoryPressure",
        "AssertNeedsUpdate",
        "AssertOSRelease",
        "AssertPathExists",
        "AssertPathExistsGlob",
        "AssertPathIsDirectory",
        "AssertPathIsEncrypted",
        "AssertPathIsMountPoint",
        "AssertPathIsReadWrite",
        "AssertPathIsSymbolicLink",
        "AssertSecurity",
        "AssertUser",
        "AssertVersion",
        "AssertVirtualization",
        "Before",
        "BindsTo",
        "CollectMode",
        "ConditionACPower",
        "ConditionArchitecture",
        "ConditionCPUFeature",
        "ConditionCPUPressure",
        "ConditionCPUs",
        "ConditionCapability",
        "ConditionControlGroupController",
        "ConditionCredential",
        "ConditionDirectoryNotEmpty",
        "ConditionEnvironment",
        "ConditionFileIsExecutable",
        "ConditionFileNotEmpty",
        "ConditionFirmware",
        "ConditionFirstBoot",
        "ConditionGroup",
        "ConditionHost",
        "ConditionIOPressure",
        "ConditionKernelCommandLine",
        "ConditionKernelModuleLoaded",
        "ConditionKernelVersion",
        "ConditionMemory",
        "ConditionMemoryPressure",
        "ConditionNeedsUpdate",
        "ConditionOSRelease",
        "ConditionPathExists",
        "ConditionPathExistsGlob",
        "ConditionPathIsDirectory",
        "ConditionPathIsEncrypted",
        "ConditionPathIsMountPoint",
        "ConditionPathIsReadWrite",
        "ConditionPathIsSymbolicLink",
        "ConditionSecurity",
        "ConditionUser",
        "ConditionVersion",
        "ConditionVirtualization",
        "Conflicts",
        "DefaultDependencies",
        "Description",
        "Documentation",
        "FailureAction",
        "FailureActionExitStatus",
        "IgnoreOnIsolate",
        "JobRunningTimeoutSec",
        "JobTimeoutAction",
        "JobTimeoutRebootArgument",
        "JobTimeoutSec",
        "JoinsNamespaceOf",
        "OnFailure",
        "OnFailureJobMode",
        "OnSuccess",
        "OnSuccessJobMode",
        "PartOf",
        "PropagatesReloadTo",
        "PropagatesStopTo",
        "RebootArgument",
        "RefuseManualStart",
        "RefuseManualStop",
        "ReloadPropagatedFrom",
        "Requires",
        "RequiresMountsFor",
        "Requisite",
        "SourcePath",
        "StartLimitAction",
        "StartLimitBurst",
        "StartLimitIntervalSec",
        "StopPropagatedFrom",
        "StopWhenUnneeded",
        "SuccessAction",
        "SuccessActionExitStatus",
        "SurviveFinalKillSignal",
        "Upholds",
        "Wants",
        "WantsMountsFor",
    };
    return keys;
}

const QStringList &installSectionKeys()
{
    static const QStringList keys{
        "Alias",
        "Also",
        "DefaultInstance",
        "RequiredBy",
        "UpheldBy",
        "WantedBy",
    };
    return keys;
}

const QStringList &serviceOwnKeys()
{
    static const QStringList keys{
        "BusName",
        "ExecCondition",
        "ExecReload",
        "ExecStart",
        "ExecStop",
        "ExitType",
        "FileDescriptorStoreMax",
        "FileDescriptorStorePreserve",
        "GuessMainPID",
        "NonBlocking",
        "NotifyAccess",
        "OpenFile",
        "PIDFile",
        "ReloadSignal",
        "RemainAfterExit",
        "Restart",
        "RestartForceExitStatus",
        "RestartMaxDelaySec",
        "RestartMode",
        "RestartPreventExitStatus",
        "RestartSec",
        "RestartSteps",
        "RootDirectoryStartOnly",
        "Sockets",
        "SuccessExitStatus",
        "TimeoutAbortSec",
        "TimeoutStartFailureMode",
        "TimeoutStartSec",
        "TimeoutStopFailureMode",
        "TimeoutStopSec",
        "USBFunctionDescriptors",
        "USBFunctionStrings",
        "WatchdogSec",
    };
    return keys;
}

const QStringList &socketOwnKeys()
{
    static const QStringList keys{
        "Accept",
        "AcceptFileDescriptors",
        "Backlog",
        "BindIPv6Only",
        "BindToDevice",
        "Broadcast",
        "DeferAcceptSec",
        "DeferTrigger",
        "DeferTriggerMaxSec",
        "ExecStopPre",
        "FileDescriptorName",
        "FlushPending",
        "FreeBind",
        "IPTOS",
        "IPTTL",
        "KeepAlive",
        "KeepAliveIntervalSec",
        "KeepAliveProbes",
        "KeepAliveTimeSec",
        "ListenDatagram",
        "ListenFIFO",
        "ListenMessageQueue",
        "ListenNetlink",
        "ListenSequentialPacket",
        "ListenSpecial",
        "ListenStream",
        "ListenUSBFunction",
        "Mark",
        "MaxConnections",
        "MaxConnectionsPerSource",
        "MessageQueueMaxMessages",
        "MessageQueueMessageSize",
        "NoDelay",
        "PassCredentials",
        "PassFileDescriptorsToExec",
        "PassPIDFD",
        "PassPacketInfo",
        "PassSecurity",
        "PipeSize",
        "PollLimitBurst",
        "PollLimitIntervalSec",
        "ReceiveBuffer",
        "RemoveOnStop",
        "ReusePort",
        "SELinuxContextFromNet",
        "SendBuffer",
        "Service",
        "SmackLabel",
        "SmackLabelIPIn",
        "SmackLabelIPOut",
        "SocketGroup",
        "SocketMode",
        "SocketProtocol",
        "SocketUser",
        "Symlinks",
        "TCPCongestion",
        "Timestamping",
        "Transparent",
        "Writable",
    };
    return keys;
}

const QStringList &timerOwnKeys()
{
    static const QStringList keys{
        "AccuracySec",
        "DeferReactivation",
        "FixedRandomDelay",
        "OnActiveSec",
        "OnBootSec",
        "OnCalendar",
        "OnClockChange",
        "OnStartupSec",
        "OnTimezoneChange",
        "OnUnitActiveSec",
        "OnUnitInactiveSec",
        "Persistent",
        "RandomizedDelaySec",
        "RandomizedOffsetSec",
        "RemainAfterElapse",
        "Unit",
        "WakeSystem",
    };
    return keys;
}

const QStringList &pathOwnKeys()
{
    static const QStringList keys{
        "DirectoryMode",
        "DirectoryNotEmpty",
        "MakeDirectory",
        "PathChanged",
        "PathExists",
        "PathExistsGlob",
        "PathModified",
        "Unit",
    };
    return keys;
}

const QStringList &mountOwnKeys()
{
    static const QStringList keys{
        "DirectoryMode",
        "ForceUnmount",
        "LazyUnmount",
        "Options",
        "ReadWriteOnly",
        "SloppyOptions",
        "TimeoutSec",
        "Type",
        "What",
        "Where",
    };
    return keys;
}

const QStringList &automountOwnKeys()
{
    static const QStringList keys{
        "DirectoryMode",
        "ExtraOptions",
        "TimeoutIdleSec",
        "Where",
    };
    return keys;
}

const QStringList &swapOwnKeys()
{
    static const QStringList keys{
        "Options",
        "Priority",
        "TimeoutSec",
        "What",
    };
    return keys;
}

const QStringList &sliceOwnKeys()
{
    static const QStringList keys{
        "ConcurrencyHardMax",
        "ConcurrencySoftMax",
    };
    return keys;
}

const QStringList &scopeOwnKeys()
{
    static const QStringList keys{
        "OOMPolicy",
        "RuntimeMaxSec",
        "RuntimeRandomizedExtraSec",
    };
    return keys;
}

const QStringList &execSharedKeys()
{
    static const QStringList keys{
        "AmbientCapabilities",
        "AppArmorProfile",
        "BPFDelegateAttachments",
        "BPFDelegateCommands",
        "BPFDelegateMaps",
        "BPFDelegatePrograms",
        "BindLogSockets",
        "BindPaths",
        "BindReadOnlyPaths",
        "CPUAffinity",
        "CPUSchedulingPolicy",
        "CPUSchedulingPriority",
        "CPUSchedulingResetOnFork",
        "CacheDirectory",
        "CacheDirectoryAccounting",
        "CacheDirectoryMode",
        "CacheDirectoryQuota",
        "CapabilityBoundingSet",
        "ConfigurationDirectory",
        "ConfigurationDirectoryMode",
        "CoredumpFilter",
        "DelegateNamespaces",
        "DynamicUser",
        "Environment",
        "EnvironmentFile",
        "ExecPaths",
        "ExecSearchPath",
        "ExtensionDirectories",
        "ExtensionImagePolicy",
        "ExtensionImages",
        "Group",
        "IOSchedulingClass",
        "IOSchedulingPriority",
        "IPCNamespacePath",
        "IgnoreSIGPIPE",
        "ImportCredential",
        "InaccessiblePaths",
        "KeyringMode",
        "LimitAS",
        "LimitCORE",
        "LimitCPU",
        "LimitDATA",
        "LimitFSIZE",
        "LimitLOCKS",
        "LimitMEMLOCK",
        "LimitMSGQUEUE",
        "LimitNICE",
        "LimitNOFILE",
        "LimitNPROC",
        "LimitRSS",
        "LimitRTPRIO",
        "LimitRTTIME",
        "LimitSIGPENDING",
        "LimitSTACK",
        "LoadCredential",
        "LoadCredentialEncrypted",
        "LockPersonality",
        "LogExtraFields",
        "LogFilterPatterns",
        "LogLevelMax",
        "LogNamespace",
        "LogRateLimitBurst",
        "LogRateLimitIntervalSec",
        "LogsDirectory",
        "LogsDirectoryAccounting",
        "LogsDirectoryMode",
        "LogsDirectoryQuota",
        "MemoryDenyWriteExecute",
        "MemoryKSM",
        "MountAPIVFS",
        "MountFlags",
        "MountImagePolicy",
        "MountImages",
        "NUMAMask",
        "NUMAPolicy",
        "NetworkNamespacePath",
        "Nice",
        "NoExecPaths",
        "NoNewPrivileges",
        "OOMScoreAdjust",
        "PAMName",
        "PassEnvironment",
        "Personality",
        "PrivateBPF",
        "PrivateDevices",
        "PrivateIPC",
        "PrivateMounts",
        "PrivateNetwork",
        "PrivatePIDs",
        "PrivateTmp",
        "PrivateUsers",
        "ProcSubset",
        "ProtectClock",
        "ProtectControlGroups",
        "ProtectHome",
        "ProtectHostname",
        "ProtectKernelLogs",
        "ProtectKernelModules",
        "ProtectKernelTunables",
        "ProtectProc",
        "ProtectSystem",
        "ReadOnlyPaths",
        "ReadWritePaths",
        "RemoveIPC",
        "RestrictAddressFamilies",
        "RestrictFileSystems",
        "RestrictNamespaces",
        "RestrictRealtime",
        "RestrictSUIDSGID",
        "RootDirectory",
        "RootEphemeral",
        "RootHash",
        "RootHashSignature",
        "RootImage",
        "RootImageOptions",
        "RootImagePolicy",
        "RootVerity",
        "RuntimeDirectory",
        "RuntimeDirectoryMode",
        "RuntimeDirectoryPreserve",
        "SELinuxContext",
        "SecureBits",
        "SetCredential",
        "SetCredentialEncrypted",
        "SetLoginEnvironment",
        "SmackProcessLabel",
        "StandardError",
        "StandardInput",
        "StandardInputData",
        "StandardInputText",
        "StandardOutput",
        "StateDirectory",
        "StateDirectoryAccounting",
        "StateDirectoryMode",
        "StateDirectoryQuota",
        "SupplementaryGroups",
        "SyslogFacility",
        "SyslogIdentifier",
        "SyslogLevel",
        "SyslogLevelPrefix",
        "SystemCallArchitectures",
        "SystemCallErrorNumber",
        "SystemCallFilter",
        "SystemCallLog",
        "TTYColumns",
        "TTYPath",
        "TTYReset",
        "TTYRows",
        "TTYVHangup",
        "TTYVTDisallocate",
        "TemporaryFileSystem",
        "TimeoutCleanSec",
        "TimerSlackNSec",
        "UMask",
        "UnsetEnvironment",
        "User",
        "UtmpIdentifier",
        "UtmpMode",
        "WorkingDirectory",
    };
    return keys;
}

const QStringList &killSharedKeys()
{
    static const QStringList keys{
        "FinalKillSignal",
        "KillMode",
        "KillSignal",
        "RestartKillSignal",
        "SendSIGHUP",
        "SendSIGKILL",
        "WatchdogSignal",
    };
    return keys;
}

const QStringList &resourceSharedKeys()
{
    static const QStringList keys{
        "AllowedCPUs",
        "AllowedMemoryNodes",
        "BPFProgram",
        "CPUQuota",
        "CPUQuotaPeriodSec",
        "CPUWeight",
        "CoredumpReceive",
        "DefaultStartupMemoryLow",
        "Delegate",
        "DelegateSubgroup",
        "DeviceAllow",
        "DevicePolicy",
        "DisableControllers",
        "IOAccounting",
        "IODeviceLatencyTargetSec",
        "IODeviceWeight",
        "IOReadBandwidthMax",
        "IOReadIOPSMax",
        "IOWeight",
        "IOWriteBandwidthMax",
        "IOWriteIOPSMax",
        "IPAccounting",
        "IPAddressAllow",
        "IPAddressDeny",
        "IPEgressFilterPath",
        "IPIngressFilterPath",
        "ManagedOOMMemoryPressure",
        "ManagedOOMMemoryPressureDurationSec",
        "ManagedOOMMemoryPressureLimit",
        "ManagedOOMPreference",
        "ManagedOOMSwap",
        "MemoryAccounting",
        "MemoryHigh",
        "MemoryLow",
        "MemoryMax",
        "MemoryMin",
        "MemoryPressureThresholdSec",
        "MemoryPressureWatch",
        "MemorySwapMax",
        "MemoryZSwapMax",
        "MemoryZSwapWriteback",
        "NFTSet",
        "RestrictNetworkInterfaces",
        "Slice",
        "SocketBindAllow",
        "SocketBindDeny",
        "StartupAllowedCPUs",
        "StartupAllowedMemoryNodes",
        "StartupCPUWeight",
        "StartupIOWeight",
        "StartupMemoryHigh",
        "StartupMemoryLow",
        "StartupMemoryMax",
        "StartupMemorySwapMax",
        "StartupMemoryZSwapMax",
        "TasksAccounting",
        "TasksMax",
    };
    return keys;
}

const QStringList &serviceSectionKeys()
{
    static const QStringList keys = mergeKeyLists({serviceOwnKeys(), execSharedKeys(), killSharedKeys(), resourceSharedKeys()});
    return keys;
}

const QStringList &socketSectionKeys()
{
    static const QStringList keys = mergeKeyLists({socketOwnKeys(), execSharedKeys(), killSharedKeys(), resourceSharedKeys()});
    return keys;
}

const QStringList &mountSectionKeys()
{
    static const QStringList keys = mergeKeyLists({mountOwnKeys(), execSharedKeys(), killSharedKeys(), resourceSharedKeys()});
    return keys;
}

const QStringList &swapSectionKeys()
{
    static const QStringList keys = mergeKeyLists({swapOwnKeys(), execSharedKeys(), killSharedKeys(), resourceSharedKeys()});
    return keys;
}

const QStringList &sliceSectionKeys()
{
    static const QStringList keys = mergeKeyLists({sliceOwnKeys(), resourceSharedKeys()});
    return keys;
}

const QStringList &scopeSectionKeys()
{
    static const QStringList keys = mergeKeyLists({scopeOwnKeys(), killSharedKeys(), resourceSharedKeys()});
    return keys;
}

MandatoryProfile profileForType(SystemdUnitType type)
{
    MandatoryProfile profile;

    switch (type)
    {
    case SystemdUnitType::Service:
        profile.oneOfGroups.push_back(MandatoryGroup{"service.exec",
                                                     "Service",
                                                     QStringList{"ExecStart", "ExecStop"},
                                                     "ExecStart"});
        profile.requireServiceRemainAfterExitWithExecStopOnly = true;
        break;
    case SystemdUnitType::Socket:
        profile.oneOfGroups.push_back(
            MandatoryGroup{"socket.listen",
                           "Socket",
                           QStringList{"ListenStream",
                                       "ListenDatagram",
                                       "ListenSequentialPacket",
                                       "ListenFIFO",
                                       "ListenSpecial",
                                       "ListenNetlink",
                                       "ListenMessageQueue",
                                       "ListenUSBFunction"},
                           "ListenStream"});
        break;
    case SystemdUnitType::Timer:
        profile.oneOfGroups.push_back(
            MandatoryGroup{"timer.schedule",
                           "Timer",
                           QStringList{"OnActiveSec",
                                       "OnBootSec",
                                       "OnStartupSec",
                                       "OnUnitActiveSec",
                                       "OnUnitInactiveSec",
                                       "OnCalendar"},
                           "OnCalendar"});
        break;
    case SystemdUnitType::Path:
        profile.oneOfGroups.push_back(
            MandatoryGroup{"path.watch",
                           "Path",
                           QStringList{"PathExists",
                                       "PathExistsGlob",
                                       "PathChanged",
                                       "PathModified",
                                       "DirectoryNotEmpty"},
                           "PathExists"});
        break;
    case SystemdUnitType::Mount:
        profile.strictKeys.push_back({"Mount", "What"});
        profile.strictKeys.push_back({"Mount", "Where"});
        break;
    case SystemdUnitType::Automount:
        profile.strictKeys.push_back({"Automount", "Where"});
        break;
    case SystemdUnitType::Swap:
        profile.strictKeys.push_back({"Swap", "What"});
        break;
    case SystemdUnitType::Target:
    case SystemdUnitType::Device:
    case SystemdUnitType::Slice:
    case SystemdUnitType::Scope:
    default:
        break;
    }

    return profile;
}

} // namespace

namespace preferences
{

QStringList SystemdManCatalog::sectionsForUnitType(SystemdUnitType type)
{
    switch (type)
    {
    case SystemdUnitType::Service:
        return {"Unit", "Service", "Install"};
    case SystemdUnitType::Socket:
        return {"Unit", "Socket", "Install"};
    case SystemdUnitType::Timer:
        return {"Unit", "Timer", "Install"};
    case SystemdUnitType::Path:
        return {"Unit", "Path", "Install"};
    case SystemdUnitType::Mount:
        return {"Unit", "Mount", "Install"};
    case SystemdUnitType::Automount:
        return {"Unit", "Automount", "Install"};
    case SystemdUnitType::Swap:
        return {"Unit", "Swap", "Install"};
    case SystemdUnitType::Target:
        return {"Unit", "Install"};
    case SystemdUnitType::Device:
        return {"Unit", "Install"};
    case SystemdUnitType::Slice:
        return {"Unit", "Slice", "Install"};
    case SystemdUnitType::Scope:
        return {"Unit", "Scope"};
    default:
        return {"Unit", "Install"};
    }
}

QStringList SystemdManCatalog::keysFor(SystemdUnitType type, const QString &section)
{
    if (equalsInsensitive(section, "Unit"))
    {
        return unitSectionKeys();
    }

    if (equalsInsensitive(section, "Install"))
    {
        return installSectionKeys();
    }

    switch (type)
    {
    case SystemdUnitType::Service:
        if (equalsInsensitive(section, "Service"))
        {
            return serviceSectionKeys();
        }
        break;
    case SystemdUnitType::Socket:
        if (equalsInsensitive(section, "Socket"))
        {
            return socketSectionKeys();
        }
        break;
    case SystemdUnitType::Timer:
        if (equalsInsensitive(section, "Timer"))
        {
            return timerOwnKeys();
        }
        break;
    case SystemdUnitType::Path:
        if (equalsInsensitive(section, "Path"))
        {
            return pathOwnKeys();
        }
        break;
    case SystemdUnitType::Mount:
        if (equalsInsensitive(section, "Mount"))
        {
            return mountSectionKeys();
        }
        break;
    case SystemdUnitType::Automount:
        if (equalsInsensitive(section, "Automount"))
        {
            return automountOwnKeys();
        }
        break;
    case SystemdUnitType::Swap:
        if (equalsInsensitive(section, "Swap"))
        {
            return swapSectionKeys();
        }
        break;
    case SystemdUnitType::Slice:
        if (equalsInsensitive(section, "Slice"))
        {
            return sliceSectionKeys();
        }
        break;
    case SystemdUnitType::Scope:
        if (equalsInsensitive(section, "Scope"))
        {
            return scopeSectionKeys();
        }
        break;
    case SystemdUnitType::Target:
    case SystemdUnitType::Device:
    default:
        break;
    }

    return {};
}

MandatoryProfile SystemdManCatalog::mandatoryProfile(SystemdUnitType type)
{
    return profileForType(type);
}

QList<MandatoryScaffoldEntry> SystemdManCatalog::mandatoryScaffold(SystemdUnitType type)
{
    const auto profile = profileForType(type);
    QList<MandatoryScaffoldEntry> scaffold;

    for (const auto &strict : profile.strictKeys)
    {
        scaffold.push_back(MandatoryScaffoldEntry{strict.first, strict.second, true, QString()});
    }

    for (const auto &group : profile.oneOfGroups)
    {
        const auto defaultKey = !group.defaultKey.isEmpty() ? group.defaultKey : group.keys.value(0);
        if (!group.section.isEmpty() && !defaultKey.isEmpty())
        {
            scaffold.push_back(MandatoryScaffoldEntry{group.section, defaultKey, false, group.id});
        }
    }

    return scaffold;
}

QPair<QString, QString> SystemdManCatalog::defaultSectionKey(SystemdUnitType type)
{
    const auto scaffold = mandatoryScaffold(type);
    if (!scaffold.isEmpty())
    {
        return {scaffold.first().section, scaffold.first().key};
    }

    for (const auto &section : sectionsForUnitType(type))
    {
        const auto keys = keysFor(type, section);
        if (!keys.isEmpty())
        {
            return {section, keys.first()};
        }
    }

    return {QStringLiteral("Unit"), QStringLiteral("Description")};
}

} // namespace preferences
