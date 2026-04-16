
#include "PoiEventSubsystem.h"
#include "JsonObjectConverter.h"
#include "Kismet/GameplayStatics.h"
#include "SimplePoi/Gameplay/PoiBasePawn.h"


AActor* UPoiActionSet::GetOuterAsActor()
{
	return Cast<AActor>(GetOuter());
}

UObject* UPoiActionSet::GetOuterObject()
{
	return GetOuter();
}

void UPoiEventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UPoiEventSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UPoiEventSubsystem::AddFocusDataToMap(const FFocusMessageStruct& FocusData)
{
	FocusMessageMap.Add(FocusData.Name,FocusData);
}

void UPoiEventSubsystem::AddPoiActorToGroup(AActor* InActor,const bool bIsAddToMessageMap)
{
	FString GroupStr;
	FString KeyStr;
	if (IPoiActorInterface::Execute_GetGroupAndKey(InActor, GroupStr, KeyStr))
	{
		if (GroupStr.IsEmpty() && !bIsAddToMessageMap)
		{
			UE_LOG(LogTemp, Warning, TEXT("AddPoiActorToGroup: GroupStr is empty for actor %s"), *InActor->GetName());
			return;
		}
		const FFocusMessageStruct FocusMessage = IPoiActorInterface::Execute_GetFocusMessage(InActor);
		if (bIsAddToMessageMap)
		{
			FocusMessageMap.Add(KeyStr,FocusMessage);
		}
		else
		{
			// 使用 FindOrAdd 获取引用,避免不必要的拷贝
			FPoiMapData& GroupData = PoiActorGroupMap.FindOrAdd(GroupStr);
			GroupData.PoiActorKeyMap.Add(KeyStr,InActor);
		}
	}
}

void UPoiEventSubsystem::FindAllActorInGroup(const FString& InGroup, TArray<AActor*>& OutActorArray)
{
	if (PoiActorGroupMap.Contains(InGroup))
	{
		TArray<TObjectPtr<AActor>> ActorArray;
		PoiActorGroupMap[InGroup].PoiActorKeyMap.GenerateValueArray(ActorArray);
		OutActorArray=ActorArray;
	}
}

AActor* UPoiEventSubsystem::FindPoiActorByGroupAndKey(const FString& InGroup, const FString& InKey)
{
	if (InGroup.IsEmpty())
	{
		for (auto Element : PoiActorGroupMap)
		{
			if (Element.Value.PoiActorKeyMap.Contains(InKey))
			{
				return Element.Value.PoiActorKeyMap.FindRef(InKey);
			}
		}
	}
	else
	{
		if (PoiActorGroupMap.Contains(InGroup))
		{
			return PoiActorGroupMap[InGroup].PoiActorKeyMap.FindRef(InKey);
		}
	}
	return nullptr;
}

bool UPoiEventSubsystem::FindFocusMessageByGroupAndKey(const FString& InGroup, const FString& InKey,FFocusMessageStruct& OutFocusMessage)
{
	if (FocusMessageMap.Contains(InKey))
	{
		OutFocusMessage=FocusMessageMap[InKey];
		return true;
	}
	else if (PoiActorGroupMap.Contains(InGroup))
	{
		if (PoiActorGroupMap[InGroup].PoiActorKeyMap.Contains(InKey))
		{
			OutFocusMessage=IPoiActorInterface::Execute_GetFocusMessage(PoiActorGroupMap[InGroup].PoiActorKeyMap.FindRef(InKey));
			return true;
		}
	}
	else
	{
		for (auto Element : PoiActorGroupMap)
		{
			if (Element.Value.PoiActorKeyMap.Contains(InKey))
			{
				OutFocusMessage=IPoiActorInterface::Execute_GetFocusMessage(Element.Value.PoiActorKeyMap.FindRef(InKey));
				return true;
			}
		}
	}
	return false;
}

void UPoiEventSubsystem::BroadcastFocusMessageByDelegate(const FFocusMessageStruct& FocusData)
{
	if (FString JsonString; PoiStructToJsonString(FocusData,JsonString))
	{
		SendActorSimpleStrByTag(nullptr,FGameplayTag::RequestGameplayTag(TEXT("SimplePoiPlugin.PoiAction.Focus")),JsonString);
	}
}

bool UPoiEventSubsystem::FocusToActorByKey(const FString& InGroup, const FString& InKey)
{
	if (FFocusMessageStruct FocusMessage; FindFocusMessageByGroupAndKey(InGroup,InKey,FocusMessage))
	{
		BroadcastFocusMessageByDelegate(FocusMessage);
		return true;
	}
	return false;
}

void UPoiEventSubsystem::FocusToActorByFocusMessage(AActor* InActor)
{
	if (InActor)
	{
		IPoiActorInterface::Execute_FocusToSelf(InActor);
	}
}

APoiBasePawn* UPoiEventSubsystem::GetPoiPawn()
{
	if (PoiBasePawn != nullptr)return PoiBasePawn;
	if (GetWorld()->GetFirstPlayerController())
	{
		PoiBasePawn = Cast<APoiBasePawn>(GetWorld()->GetFirstPlayerController()->GetPawn());
	}
	// 如果玩家控制器没有 APoiBasePawn，则在整个世界中查找
	if (GetWorld())
	{
		TArray<AActor*> PawnActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APoiBasePawn::StaticClass(), PawnActors);
        
		// 如果找到 APoiBasePawn 类型的 actor，使用第一个
		if (PawnActors.Num() > 0)
		{
			PoiBasePawn = Cast<APoiBasePawn>(PawnActors[0]);
			return PoiBasePawn;
		}
	}
	return nullptr;
}

void UPoiEventSubsystem::SetCurrentSelectedActor(AActor* InActor)
{
	if (InActor == nullptr)
	{
		IPoiActorInterface::Execute_EUnSelected(CurrentSelectedActor);
		CurrentSelectedActor = nullptr;
		return;
	}
	if (CurrentSelectedActor && CurrentSelectedActor != InActor)
	{
		IPoiActorInterface::Execute_EUnSelected(CurrentSelectedActor);
	}
	CurrentSelectedActor = InActor;
	IPoiActorInterface::Execute_ESelected(CurrentSelectedActor);
}

void UPoiEventSubsystem::SendActorStrByTag(AActor* CurActor, FGameplayTag CommandTag, const FPoiMessageStruct& InMessage)
{
	ReceivedActorStrByTag.Broadcast(CurActor,CommandTag,InMessage);
}

void UPoiEventSubsystem::SendObjectStrByTag(UObject* CurObject, FGameplayTag CommandTag, const FPoiMessageStruct& InMessage)
{
	ReceivedObjectStrByTag.Broadcast(CurObject,CommandTag,InMessage);
}

void UPoiEventSubsystem::SendActorSimpleStrByTag(AActor* CurActor, FGameplayTag CommandTag, const FString& InMessage)
{
	ReceivedActorSimpleStrByTag.Broadcast(CurActor,CommandTag,InMessage);
}

void UPoiEventSubsystem::SendObjectSimpleStrByTag(UObject* CurObject, FGameplayTag CommandTag, const FString& InMessage)
{
	ReceivedObjectSimpleStrByTag.Broadcast(CurObject,CommandTag,InMessage);
}

bool UPoiEventSubsystem::JsonStringToPoiStruct(const FString JsonString, FFocusMessageStruct& OutStruct)
{
	if (FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &OutStruct))
	{
		//UE_LOG(LogConfig, Display, TEXT("Json转换为结构体数据成功"));
		return true;
	}
	return false;
}

bool UPoiEventSubsystem::PoiStructToJsonString(const FFocusMessageStruct& InStruct, FString& OutJsonString)
{
	return (FJsonObjectConverter::UStructToJsonObjectString(InStruct, OutJsonString));
}

FString UPoiEventSubsystem::FocusMessageMapToJsonString()
{
	
    // 创建JSON对象
    TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);
    
    // 遍历映射表，将每个键值对转换为JSON
    for (const auto& MapPair : FocusMessageMap)
    {
        // 将FFocusMessageStruct结构体转换为JSON对象
        TSharedPtr<FJsonObject> FocusMsgObj = MakeShareable(new FJsonObject);
        FocusMsgObj->SetStringField(TEXT("Name"), MapPair.Value.Name);
        FocusMsgObj->SetBoolField(TEXT("bIsTeleport"), MapPair.Value.bIsTeleport);
        
        // 设置位置向量
        TSharedPtr<FJsonObject> LocationObj = MakeShareable(new FJsonObject);
        LocationObj->SetNumberField(TEXT("X"), MapPair.Value.TargetLocation.X);
        LocationObj->SetNumberField(TEXT("Y"), MapPair.Value.TargetLocation.Y);
        LocationObj->SetNumberField(TEXT("Z"), MapPair.Value.TargetLocation.Z);
        FocusMsgObj->SetObjectField(TEXT("TargetLocation"), LocationObj);
        
        // 设置旋转角度
        TSharedPtr<FJsonObject> RotationObj = MakeShareable(new FJsonObject);
        RotationObj->SetNumberField(TEXT("Pitch"), MapPair.Value.TargetRotation.Pitch);
        RotationObj->SetNumberField(TEXT("Yaw"), MapPair.Value.TargetRotation.Yaw);
        RotationObj->SetNumberField(TEXT("Roll"), MapPair.Value.TargetRotation.Roll);
        FocusMsgObj->SetObjectField(TEXT("TargetRotation"), RotationObj);
        
        FocusMsgObj->SetNumberField(TEXT("TargetSprintLens"), MapPair.Value.TargetSprintLens);
        FocusMsgObj->SetNumberField(TEXT("FocusTimes"), MapPair.Value.FocusTimes);
        
        // 将JSON对象添加到根对象中
        RootObject->SetObjectField(MapPair.Key, FocusMsgObj);
    }
    
    // 将JSON对象转换为字符串
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);
    
    return OutputString;
}
FString UPoiEventSubsystem::PoiActorGroupMapToJsonString()
{
    // 创建JSON对象
    TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);
    
    // 遍历分组映射表
    for (const auto& GroupPair : PoiActorGroupMap)
    {
        // 为每个分组创建JSON数组
        TArray<TSharedPtr<FJsonValue>> ActorArray;
        
        // 遍历该分组下的所有Actor
        for (const auto& ActorPair : GroupPair.Value.PoiActorKeyMap)
        {
            // 创建Actor信息对象
            TSharedPtr<FJsonObject> ActorObj = MakeShareable(new FJsonObject);
            
            // 获取Actor名称作为标识
            FString ActorName = ActorPair.Value ? ActorPair.Value->GetName() : TEXT("Null");
            ActorObj->SetStringField(TEXT("ActorName"), ActorName);
            ActorObj->SetStringField(TEXT("Key"), ActorPair.Key);
            
            // 尝试获取Actor的聚焦信息
            if (ActorPair.Value && ActorPair.Value->GetClass()->ImplementsInterface(UPoiActorInterface::StaticClass()))
            {
                FFocusMessageStruct FocusMsg = IPoiActorInterface::Execute_GetFocusMessage(ActorPair.Value);
                
                // 添加聚焦信息到Actor对象
                TSharedPtr<FJsonObject> FocusMsgObj = MakeShareable(new FJsonObject);
                FocusMsgObj->SetStringField(TEXT("Name"), FocusMsg.Name);
                FocusMsgObj->SetBoolField(TEXT("bIsTeleport"), FocusMsg.bIsTeleport);
                
                // 设置位置向量
                TSharedPtr<FJsonObject> LocationObj = MakeShareable(new FJsonObject);
                LocationObj->SetNumberField(TEXT("X"), FocusMsg.TargetLocation.X);
                LocationObj->SetNumberField(TEXT("Y"), FocusMsg.TargetLocation.Y);
                LocationObj->SetNumberField(TEXT("Z"), FocusMsg.TargetLocation.Z);
                FocusMsgObj->SetObjectField(TEXT("TargetLocation"), LocationObj);
                
                // 设置旋转角度
                TSharedPtr<FJsonObject> RotationObj = MakeShareable(new FJsonObject);
                RotationObj->SetNumberField(TEXT("Pitch"), FocusMsg.TargetRotation.Pitch);
                RotationObj->SetNumberField(TEXT("Yaw"), FocusMsg.TargetRotation.Yaw);
                RotationObj->SetNumberField(TEXT("Roll"), FocusMsg.TargetRotation.Roll);
                FocusMsgObj->SetObjectField(TEXT("TargetRotation"), RotationObj);
                
                FocusMsgObj->SetNumberField(TEXT("TargetSprintLens"), FocusMsg.TargetSprintLens);
                FocusMsgObj->SetNumberField(TEXT("FocusTimes"), FocusMsg.FocusTimes);
                
                ActorObj->SetObjectField(TEXT("FocusMessage"), FocusMsgObj);
            }
            
            ActorArray.Add(MakeShareable(new FJsonValueObject(ActorObj)));
        }
        
        // 将Actor数组添加到分组对象中
        RootObject->SetArrayField(GroupPair.Key, ActorArray);
    }
    
    // 将JSON对象转换为字符串
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);
    
    return OutputString;
}


bool UPoiEventSubsystem::FindAniSpeed(const FString& AniKeyCut, const FWidgetDataStruct& WidgetData, float& OutSpeed)
{
	TArray<FString> Keys;
	WidgetData.AniSpeedMap.GetKeys(Keys);
	for (const FString& AniKey : Keys)
	{
		if (AniKey.Contains(AniKeyCut))
		{
			OutSpeed = WidgetData.AniSpeedMap[AniKey];
			return true;
		}
	}
	return false;
}
