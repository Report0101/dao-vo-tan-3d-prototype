using UnrealBuildTool;
public class DaoVoTan3DEditorTarget : TargetRules
{
    public DaoVoTan3DEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        ExtraModuleNames.Add("DaoVoTan3D");
    }
}
