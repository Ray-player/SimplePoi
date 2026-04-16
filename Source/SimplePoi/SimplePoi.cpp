// Copyright Epic Games, Inc. All Rights Reserved.

#include "SimplePoi.h"

#include "GameplayTagsManager.h"

#define LOCTEXT_NAMESPACE "FSimplePoiModule"

void FSimplePoiModule::StartupModule()
{
	// 注册插件的 Tags 目录到 GameplayTagsManager 的搜索路径中
	UGameplayTagsManager::Get().AddTagIniSearchPath(FPaths::ProjectPluginsDir() / TEXT("SimplePoi/Config/Tags"));
}

void FSimplePoiModule::ShutdownModule()
{
	
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSimplePoiModule, SimplePoi)