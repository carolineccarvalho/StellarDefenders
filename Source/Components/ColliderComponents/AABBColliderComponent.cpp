//
// Created by Lucas N. Ferreira on 28/09/23.
//

#include "AABBColliderComponent.h"
#include "../../Actors/Actor.h"
#include "../../Game.h"

AABBColliderComponent::AABBColliderComponent(
    Actor* owner,
    int dx, int dy,
    int w, int h,
    ColliderLayer layer,
    bool isStatic,
    int updateOrder
):
    Component(owner, updateOrder),
    mOffset(glm::vec2((float)dx, (float)dy)),
    mWidth(w),
    mHeight(h),
    mIsStatic(isStatic),
    mLayer(layer)
{
//    mOwner->GetGame()->AddCollider(this);
}

AABBColliderComponent::~AABBColliderComponent()
{
//    mOwner->GetGame()->RemoveCollider(this);
}

glm::vec2 AABBColliderComponent::GetMin() const
{
    return mOwner->GetPosition() + mOffset;
}

glm::vec2 AABBColliderComponent::GetMax() const
{
    return GetMin() + glm::vec2((float)mWidth, (float)mHeight);
}

glm::vec2 AABBColliderComponent::GetCenter() const
{
    return GetMin() + glm::vec2((float)mWidth / 2.0f, (float)mHeight / 2.0f);
}

bool AABBColliderComponent::Intersect(const AABBColliderComponent& b) const
{
    return (GetMin().x < b.GetMax().x && GetMax().x > b.GetMin().x &&
            GetMin().y < b.GetMax().y && GetMax().y > b.GetMin().y);
}

float AABBColliderComponent::GetMinVerticalOverlap(AABBColliderComponent* b) const
{
    float top = GetMin().y - b->GetMax().y; // Top
    float down = GetMax().y - b->GetMin().y; // Down

    return (Math::Abs(top) < Math::Abs(down)) ? top : down;
}

float AABBColliderComponent::GetMinHorizontalOverlap(AABBColliderComponent* b) const
{
    float right = GetMax().x - b->GetMin().x; // Right
    float left = GetMin().x - b->GetMax().x; // Left

    return (Math::Abs(left) < Math::Abs(right)) ? left : right;
}

float AABBColliderComponent::DetectHorizontalCollision(RigidBodyComponent *rigidBody)
{
    if (mIsStatic || !mIsEnabled) return false;

    // Use spatial hashing to get nearby colliders
    auto colliders = mOwner->GetGame()->GetNearbyColliders(mOwner->GetPosition());

    std::sort(colliders.begin(), colliders.end(), [this](AABBColliderComponent* a, AABBColliderComponent* b) {
        glm::vec2 distA = a->GetCenter() - GetCenter();
        glm::vec2 distB = b->GetCenter() - GetCenter();
        return glm::dot(distA, distA) < glm::dot(distB, distB);
    });

    for (auto& collider : colliders)
    {
        if (collider == this || !collider->IsEnabled()) continue;

        // Check if the collider is in the same layer or if it should be ignored
        if (ColliderIgnoreMap.at(mLayer).find(collider->GetLayer()) != ColliderIgnoreMap.at(mLayer).end())
        {
            continue; // Ignore this collider
        }

        if (Intersect(*collider))
        {
            if (collider->GetLayer() == ColliderLayer::Collectable)
            {
                mOwner->OnHorizontalCollision(0.0f, collider);
                continue;
            }

            float minHorizontalOverlap = GetMinHorizontalOverlap(collider);
            ResolveHorizontalCollisions(rigidBody, minHorizontalOverlap);

            mOwner->OnHorizontalCollision(minHorizontalOverlap, collider);
            return minHorizontalOverlap;
        }
    }

    return 0.0f;
}

float AABBColliderComponent::DetectVertialCollision(RigidBodyComponent *rigidBody)
{
    if (mIsStatic || !mIsEnabled) return false;

    // Use spatial hashing to get nearby colliders
    auto colliders = mOwner->GetGame()->GetNearbyColliders(mOwner->GetPosition());

    std::sort(colliders.begin(), colliders.end(), [this](AABBColliderComponent* a, AABBColliderComponent* b) {
        glm::vec2 distA = a->GetCenter() - GetCenter();
        glm::vec2 distB = b->GetCenter() - GetCenter();
        return glm::dot(distA, distA) < glm::dot(distB, distB);
    });

    for (auto& collider : colliders)
    {
        if (collider == this || !collider->IsEnabled()) continue;

        // Check if the collider is in the same layer or if it should be ignored
        if (ColliderIgnoreMap.at(mLayer).find(collider->GetLayer()) != ColliderIgnoreMap.at(mLayer).end())
        {
            continue; // Ignore this collider
        }

        if (Intersect(*collider))
        {
            if (collider->GetLayer() == ColliderLayer::Collectable)
            {
                mOwner->OnVerticalCollision(0.0f, collider);
                continue;
            }

            float minVerticalOverlap = GetMinVerticalOverlap(collider);
            ResolveVerticalCollisions(rigidBody, minVerticalOverlap);

            // Callback only for closest (first) collision
            mOwner->OnVerticalCollision(minVerticalOverlap, collider);
            return minVerticalOverlap;
        }
    }

    return 0.0f;
}

void AABBColliderComponent::ResolveHorizontalCollisions(RigidBodyComponent *rigidBody, const float minXOverlap)
{
    mOwner->SetPosition(mOwner->GetPosition() - glm::vec2(minXOverlap, 0.0f));
    rigidBody->SetVelocity(glm::vec2(0.f, rigidBody->GetVelocity().y));
}

void AABBColliderComponent::ResolveVerticalCollisions(RigidBodyComponent *rigidBody, const float minYOverlap)
{
    mOwner->SetPosition(mOwner->GetPosition() - glm::vec2(0.0f, minYOverlap));
    rigidBody->SetVelocity(glm::vec2(rigidBody->GetVelocity().x, 0.f));

    if (minYOverlap > .0f) {
        mOwner->SetOnGround();
    }
}