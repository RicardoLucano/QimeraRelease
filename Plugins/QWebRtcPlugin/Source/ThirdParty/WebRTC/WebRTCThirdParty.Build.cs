using System.IO;
using UnrealBuildTool;

public class WebRTCThirdParty : ModuleRules
{
	public WebRTCThirdParty(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// Add include paths
			PublicSystemIncludePaths.Add(Path.Combine(ModuleDirectory, "Include"));

			// Link import library
			string LibPath = Path.Combine(ModuleDirectory, "Lib", "win64");
			PublicAdditionalLibraries.Add(Path.Combine(LibPath, "webrtc_c_wrapper.lib"));

			// Add DLL as runtime dependency
			string DllPath = Path.Combine(LibPath, "webrtc_c_wrapper.dll");
			RuntimeDependencies.Add("$(BinaryOutputDir)/webrtc_c_wrapper.dll", DllPath);
		}
	}
}
