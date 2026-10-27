
#include "System/C4LocalPlayer.h"
#include "System/C4GameInstance.h"

FString UC4LocalPlayer::GetNickname() const
{
	if (const UC4GameInstance* GI = Cast<UC4GameInstance>(GetGameInstance()))
	{
		if (!GI->GetNickname().IsEmpty())
		{
			return GI->GetNickname();
		}
	}
	return Super::GetNickname();
}