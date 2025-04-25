#include "PhysicsSystem.h"

#include "HitResult.h"
#include "Components/UShapeComponent.h"

TArray<UShapeComponent*> FPhysicsSystem::ShapeComponents = {};

void FPhysicsSystem::RegisterComponent(UShapeComponent* InShapeComponent)
{
    ShapeComponents.Add(InShapeComponent);
}

void FPhysicsSystem::UnRegisterComponent(UShapeComponent* InShapeComponent)
{
    ShapeComponents.Remove(InShapeComponent);
}

void FPhysicsSystem::UpdateCollisions()
{
    int ShapeComponentSize = ShapeComponents.Num();  
    for(int i = 0; i < ShapeComponentSize; ++i)  
        for(int j = i + 1; j < ShapeComponentSize; ++j)  
        {  
            auto* A = ShapeComponents[i];  
            auto* B = ShapeComponents[j];  
            if(!A || !B) continue;  

            // Narrow-phase: 간단 박스 vs 박스 검사  
            FHitResult Hit;  
            if(A->TestOverlap(B, Hit))  
            {  
                // Overlap 이벤트  
                if(A->GetGeneratedOverlapEvents() && B->GetGeneratedOverlapEvents())  
                    A->OnComponentBeginOverlap.Broadcast(A, Hit.HitActor);  

                // Block 이벤트  
                if(A->bBlockComponent)  
                    A->OnComponentHit.Broadcast(A, Hit.HitActor);  
            }  
            // 반대 방향도 체크  
            if(B->TestOverlap(A, Hit))  
            {  
                if(B->GetGeneratedOverlapEvents() && A->GetGeneratedOverlapEvents())  
                    B->OnComponentBeginOverlap.Broadcast(B, Hit.HitActor);  
                if(B->bBlockComponent)  
                    B->OnComponentHit.Broadcast(B, Hit.HitActor);  
            }  
        }  
    
}
