using UnrealBuildTool;
public class DaoVoTan3D : ModuleRules {
 public DaoVoTan3D(ReadOnlyTargetRules Target) : base(Target) {
  PCHUsage=PCHUsageMode.UseExplicitOrSharedPCHs;
  PublicDependencyModuleNames.AddRange(new string[]{"Core","CoreUObject","Engine","InputCore","EnhancedInput","Landscape","Json"});
  if(Target.bBuildEditor) PrivateDependencyModuleNames.AddRange(new string[]{"UnrealEd","AssetRegistry","AssetTools"});
 }
}
