using UnrealBuildTool;
using System.Collections.Generic;

public class DaoVoTan3DTarget : TargetRules
{
    public DaoVoTan3DTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
        ExtraModuleNames.Add("DaoVoTan3D");
    }
}
