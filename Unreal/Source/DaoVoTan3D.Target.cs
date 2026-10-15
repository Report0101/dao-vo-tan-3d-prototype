using UnrealBuildTool;
public class DaoVoTan3DTarget : TargetRules
{
    public DaoVoTan3DTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        ExtraModuleNames.Add("DaoVoTan3D");
    }
}
