using UnrealBuildTool;
public class DaoVoTan3DTarget : TargetRules
{
    public DaoVoTan3DTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        ExtraModuleNames.Add("DaoVoTan3D");
    }
}
