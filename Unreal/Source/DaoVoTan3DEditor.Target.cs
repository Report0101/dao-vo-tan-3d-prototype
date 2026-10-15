using UnrealBuildTool;
using System.Collections.Generic;

public class DaoVoTan3DEditorTarget : TargetRules
{
    public DaoVoTan3DEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
        ExtraModuleNames.Add("DaoVoTan3D");
    }
}
