---@meta UnrealLuaBindings

--==============================================================================
-- Forward Declarations (Useful for complex dependencies)
--==============================================================================
---@class FString
---@class FName
---@class FVector2D
---@class FVector
---@class FRotator
---@class FQuat
---@class FMatrix
---@class UObject
---@class UClass : UObject
---@class UWorld : UObject
---@class AActor : UObject
---@class UActorComponent : UObject
---@class USceneComponent : UActorComponent
---@class EWorldType
---@class ESearchCase
---@class ESearchDir
---@class EEndPlayReason
local FString, FName, FVector2D, FVector, FRotator, FQuat, FMatrix, UObject, UClass, UWorld, AActor, UActorComponent, USceneComponent, EWorldType, ESearchCase, ESearchDir, EEndPlayReason

--==============================================================================
-- Enums
--==============================================================================

---@alias EWorldType integer
---@enum EWorldType
EWorldType = {
    None = 0,
    Game = 1,
    Editor = 2,
    PIE = 3,
    EditorPreview = 4,
    GamePreview = 5,
    Inactive = 6
}

---@alias ESearchCase integer
---@enum ESearchCase
ESearchCase = {
    CaseSensitive = 0,
    IgnoreCase = 1
}

---@alias ESearchDir integer
---@enum ESearchDir
ESearchDir = {
    FromStart = 0,
    FromEnd = 1
}

---@alias EEndPlayReason integer
---@enum EEndPlayReason
EEndPlayReason = {
    EndedPlay = 0,
    Destroyed = 1,
    RemovedFromWorld = 2,
}

---@type FName
NAME_None = nil

--==============================================================================
-- FString
--==============================================================================

---@class FString
---@field public Len fun(self: FString): integer
---@field public IsEmpty fun(self: FString): boolean
---@field public Empty fun(self: FString)
---@field public Equals fun(self: FString, other: FString, searchCase: ESearchCase): boolean
---@field public Contains fun(self: FString, subStr: FString | string, searchCase: ESearchCase, searchDir: ESearchDir): boolean
---@field public Find fun(self: FString, subStr: FString | string, searchCase: ESearchCase, searchDir: ESearchDir, startPos: integer?): integer
---@field public Reserve fun(self: FString, size: integer)
---@field public Resize fun(self: FString, newSize: integer)
---@field public ToUpper fun(self: FString): FString
---@field public ToLower fun(self: FString): FString
---@field public ToUpperInline fun(self: FString)
---@field public ToLowerInline fun(self: FString)
---@field public ToBool fun(self: FString): boolean
---@field public RightChop fun(self: FString, count: integer): FString
---@field public Append fun(self: FString, other: FString | string)
---@field public CharAt fun(self: FString, index: integer): string
---@field public __add fun(self: FString, other: FString | string): FString
---@field public __eq fun(self: FString, other: FString): boolean
---@field public __len fun(self: FString): integer
---@field public __tostring fun(self: FString): string
FString = {}

---@overload fun(): FString
---@overload fun(str: string): FString
---@overload fun(wstr: string): FString
---@return FString
function FString.new(...) end

---@param str FString
---@return integer
function FString.ToInt(str) end

---@param str FString
---@return number
function FString.ToFloat(str) end

---@overload fun(i: integer): FString
---@overload fun(ll: integer): FString
---@return FString
function FString.FromInt(...) end

---@param floatStr FString
---@return FString
function FString.SanitizeFloat(floatStr) end


--==============================================================================
-- FName
--==============================================================================

---@class FName
---@field public ToString fun(self: FName): string
---@field public IsNone fun(self: FName): boolean
---@field public __eq fun(self: FName, other: FName): boolean
---@field public __tostring fun(self: FName): string
FName = {}

---@overload fun(): FName
---@overload fun(str: string): FName
---@overload fun(fstr: FString): FName
---@return FName
function FName.new(...) end

---@type FName
FName.None = nil

--==============================================================================
-- FVector2D
--==============================================================================

---@class FVector2D
---@field X number
---@field Y number
---@field public ToString fun(self: FVector2D): string
---@field public InitFromString fun(self: FVector2D, str: FString | string): boolean
---@field public __add fun(self: FVector2D, other: FVector2D): FVector2D
---@field public __sub fun(self: FVector2D, other: FVector2D): FVector2D
---@field public __mul fun(self: FVector2D, scale: number): FVector2D
---@field public __div fun(self: FVector2D, scale: number): FVector2D
---@field public __eq fun(self: FVector2D, other: FVector2D): boolean
---@field public __tostring fun(self: FVector2D): string
FVector2D = {}

---@overload fun(): FVector2D
---@overload fun(x: number, y: number): FVector2D
---@overload fun(val: number): FVector2D
---@return FVector2D
function FVector2D.new(...) end

---@type FVector2D
FVector2D.ZeroVector = nil
---@type FVector2D
FVector2D.OneVector = nil

--==============================================================================
-- FVector
--==============================================================================

---@class FVector
---@field X number
---@field Y number
---@field Z number
---@field public Dot fun(self: FVector, other: FVector): number
---@field public Cross fun(self: FVector, other: FVector): FVector
---@field public Equals fun(self: FVector, other: FVector, tolerance: number): boolean
---@field public AllComponentsEqual fun(self: FVector, other: FVector, tolerance: number): boolean
---@field public Length fun(self: FVector): number
---@field public LengthSquared fun(self: FVector): number
---@field public Normalize fun(self: FVector, tolerance: number?): boolean
---@field public GetUnsafeNormal fun(self: FVector): FVector
---@field public GetSafeNormal fun(self: FVector, tolerance: number?): FVector
---@field public ComponentMin fun(self: FVector, other: FVector): FVector
---@field public ComponentMax fun(self: FVector, other: FVector): FVector
---@field public IsNearlyZero fun(self: FVector, tolerance: number?): boolean
---@field public IsZero fun(self: FVector): boolean
---@field public ToString fun(self: FVector): string
---@field public InitFromString fun(self: FVector, str: FString | string): boolean
---@field public __add fun(self: FVector, other: FVector): FVector
---@field public __sub fun(self: FVector, other: FVector): FVector
---@field public __mul fun(self: FVector, other: FVector | number): FVector
---@field public __div fun(self: FVector, other: FVector | number): FVector
---@field public __unm fun(self: FVector): FVector
---@field public __eq fun(self: FVector, other: FVector): boolean
---@field public __tostring fun(self: FVector): string
FVector = {}

---@overload fun(): FVector
---@overload fun(x: number, y: number, z: number): FVector
---@overload fun(val: number): FVector
---@overload fun(rot: FRotator): FVector
---@return FVector
function FVector.new(...) end

---@type FVector
FVector.ZeroVector = nil
---@type FVector
FVector.OneVector = nil
---@type FVector
FVector.UpVector = nil
---@type FVector
FVector.DownVector = nil
---@type FVector
FVector.ForwardVector = nil
---@type FVector
FVector.BackwardVector = nil
---@type FVector
FVector.RightVector = nil
---@type FVector
FVector.LeftVector = nil
---@type FVector
FVector.XAxisVector = nil
---@type FVector
FVector.YAxisVector = nil
---@type FVector
FVector.ZAxisVector = nil

---@return FVector
function FVector.Zero() end
---@return FVector
function FVector.One() end
---@return FVector
function FVector.UnitX() end
---@return FVector
function FVector.UnitY() end
---@return FVector
function FVector.UnitZ() end
---@param v1 FVector
---@param v2 FVector
---@return number
function FVector.Distance(v1, v2) end
---@param v1 FVector
---@param v2 FVector
---@return number
function FVector.DotProduct(v1, v2) end
---@param v1 FVector
---@param v2 FVector
---@return FVector
function FVector.CrossProduct(v1, v2) end
---@param v FVector
---@return FVector
function FVector.GetAbs(v) end

--==============================================================================
-- FRotator
--==============================================================================

---@class FRotator
---@field Pitch number
---@field Yaw number
---@field Roll number
---@field public IsNearlyZero fun(self: FRotator, tolerance: number?): boolean
---@field public IsZero fun(self: FRotator): boolean
---@field public Equals fun(self: FRotator, other: FRotator, tolerance: number?): boolean
---@field public Add fun(self: FRotator, deltaPitch: number, deltaYaw: number, deltaRoll: number): FRotator
---@field public FromQuaternion fun(self: FRotator, quat: FQuat): FRotator
---@field public ToQuaternion fun(self: FRotator): FQuat
---@field public ToVector fun(self: FRotator): FVector
---@field public ToMatrix fun(self: FRotator): FMatrix
---@field public Clamp fun(self: FRotator): FRotator
---@field public GetNormalized fun(self: FRotator): FRotator
---@field public Normalize fun(self: FRotator)
---@field public ToString fun(self: FRotator): string
---@field public InitFromString fun(self: FRotator, str: FString | string): boolean
---@field public __add fun(self: FRotator, other: FRotator): FRotator
---@field public __sub fun(self: FRotator, other: FRotator): FRotator
---@field public __mul fun(self: FRotator, scale: number): FRotator
---@field public __div fun(self: FRotator, other: FRotator | number): FRotator
---@field public __unm fun(self: FRotator): FRotator
---@field public __eq fun(self: FRotator, other: FRotator): boolean
---@field public __tostring fun(self: FRotator): string
FRotator = {}

---@overload fun(): FRotator
---@overload fun(pitch: number, yaw: number, roll: number): FRotator
---@overload fun(rot: FRotator): FRotator
---@overload fun(vec: FVector): FRotator
---@overload fun(quat: FQuat): FRotator
---@return FRotator
function FRotator.new(...) end


--==============================================================================
-- FQuat
--==============================================================================

---@class FQuat
---@field X number
---@field Y number
---@field Z number
---@field W number
---@field public RotateVector fun(self: FQuat, v: FVector): FVector
---@field public IsNormalized fun(self: FQuat, tolerance: number?): boolean
---@field public Normalize fun(self: FQuat, tolerance: number?)
---@field public ToMatrix fun(self: FQuat): FMatrix
---@field public __mul fun(self: FQuat, other: FQuat): FQuat
FQuat = {}

---@overload fun(): FQuat
---@overload fun(x: number, y: number, z: number, w: number): FQuat
---@overload fun(axis: FVector, angleRad: number): FQuat
---@overload fun(m: FMatrix): FQuat
---@return FQuat
function FQuat.new(...) end

---@param axis FVector
---@param angleRad number
---@return FQuat
function FQuat.FromAxisAngle(axis, angleRad) end

---@param fromVec FVector
---@param toVec FVector
---@return FQuat
function FQuat.CreateRotation(fromVec, toVec) end


--==============================================================================
-- FMatrix
--==============================================================================

---@class FMatrix
---@field public Get fun(self: FMatrix, row: integer, col: integer): number
---@field public Set fun(self: FMatrix, row: integer, col: integer, value: number)
---@field public Transpose fun(self: FMatrix): FMatrix
---@field public Inverse fun(self: FMatrix): FMatrix
---@field public TransformPosition fun(self: FMatrix, v: FVector): FVector
---@field public TransformVector fun(self: FMatrix, v: FVector): FVector
---@field public TransformFVector4 fun(self: FMatrix, v4: any): any
---@field public ToQuat fun(self: FMatrix): FQuat
---@field public __add fun(self: FMatrix, other: FMatrix): FMatrix
---@field public __sub fun(self: FMatrix, other: FMatrix): FMatrix
---@field public __mul fun(self: FMatrix, other: FMatrix | number): FMatrix
---@field public __div fun(self: FMatrix, scale: number): FMatrix
FMatrix = {}

---@type FMatrix
FMatrix.Identity = nil

---@overload fun(pitch: number, yaw: number, roll: number): FMatrix
---@overload fun(rot: FRotator): FMatrix
---@overload fun(quat: FQuat): FMatrix
---@return FMatrix
function FMatrix.CreateRotationMatrix(...) end

---@overload fun(x: number, y: number, z: number): FMatrix
---@overload fun(scaleVec: FVector): FMatrix
---@return FMatrix
function FMatrix.CreateScaleMatrix(...) end

---@overload fun(translation: FVector): FMatrix
---@return FMatrix
function FMatrix.CreateTranslationMatrix(...) end

---@param v FVector
---@param m FMatrix
---@return FVector
function FMatrix.TransformVector(v, m) end

---@param v4 any
---@param m FMatrix
---@return any
function FMatrix.TransformFVector4(v4, m) end


--==============================================================================
-- UObject
--==============================================================================

---@class UObject
---@field public Duplicate fun(self: UObject, outer: UObject?, name: FName?): UObject | nil
---@field public GetOuter fun(self: UObject): UObject | nil
---@field public GetWorld fun(self: UObject): UWorld | nil
---@field public GetFName fun(self: UObject): FName
---@field public GetName fun(self: UObject): string
---@field public GetUUID fun(self: UObject): any
---@field public GetInternalIndex fun(self: UObject): integer
---@field public GetClass fun(self: UObject): UClass | nil
---@field public IsA fun(self: UObject, classToCheck: UClass): boolean
UObject = {}

---@return UClass | nil
function UObject.StaticClass() end

---@param obj UObject
---@return any
function UObject.EncodeUUID(obj) end

--==============================================================================
-- UClass
--==============================================================================

---@class UClass : UObject
---@field public GetClassSize fun(self: UClass): integer
---@field public GetClassAlignment fun(self: UClass): integer
---@field public IsChildOf fun(self: UClass, classToCheck: UClass): boolean
---@field public GetSuperClass fun(self: UClass): UClass | nil
---@field public GetDefaultObject fun(self: UClass): UObject | nil
---@field public GetName fun(self: UClass): string
UClass = {}

---@param className string
---@return UClass | nil
function FindClass(className) end


--==============================================================================
-- UWorld
--==============================================================================

---@class UWorld : UObject
---@field WorldType EWorldType
---@field public SpawnActor fun(self: UWorld, actorClass: UClass, location: FVector?, rotation: FRotator?, name: FName?): AActor | nil
---@overload fun(self: UWorld, actorClass: UClass): AActor | nil
---@overload fun(self: UWorld, actorClass: UClass, name: FName): AActor | nil
---@overload fun(self: UWorld, actorClass: UClass, location: FVector, rotation: FRotator): AActor | nil
---@field public DestroyActor fun(self: UWorld, actorToDestroy: AActor, netForce: boolean?, shouldModifyLevel: boolean?): boolean
---@field public GetWorld fun(self: UWorld): UWorld
---@field public GetActiveLevel fun(self: UWorld): any
---@field public GetWorldName fun(self: UWorld): string
UWorld = {}

---@type UWorld | nil
GWorld = nil

--==============================================================================
-- AActor
--==============================================================================

---@class AActor : UObject
---@field public Destroy fun(self: AActor, netForce: boolean?, shouldModifyLevel: boolean?): boolean
---@field public IsActorBeingDestroyed fun(self: AActor): boolean
---@field public AddComponent fun(self: AActor, componentClass: UClass, name: FName, bManualAttachment: boolean?): UActorComponent | nil
---@field public GetRootComponent fun(self: AActor): USceneComponent | nil
---@field public SetRootComponent fun(self: AActor, newRootComponent: USceneComponent, bMaintainWorldTransform: boolean?)
---@field public GetOwner fun(self: AActor): AActor | nil
---@field public SetOwner fun(self: AActor, newOwner: AActor)
---@field public GetActorLocation fun(self: AActor): FVector
---@field public GetActorRotation fun(self: AActor): FRotator
---@field public GetActorScale fun(self: AActor): FVector
---@field public SetActorLocation fun(self: AActor, newLocation: FVector, bSweep: boolean?, teleport: integer?)
---@field public SetActorRotation fun(self: AActor, newRotation: FRotator, teleport: integer?)
---@field public SetActorScale fun(self: AActor, newScale: FVector)
---@field public GetActorForwardVector fun(self: AActor): FVector
---@field public GetActorRightVector fun(self: AActor): FVector
---@field public GetActorUpVector fun(self: AActor): FVector
---@field public GetActorLabel fun(self: AActor): string
---@field public SetActorLabel fun(self: AActor, newLabel: string, bMarkDirty: boolean?)
---@field public IsActorTickInEditor fun(self: AActor): boolean
---@field public SetActorTickInEditor fun(self: AActor, bShouldTick: boolean)
---@field public BeginPlay fun(self: AActor)
---@field public Tick fun(self: AActor, deltaTime: number)
---@field public EndPlay fun(self: AActor, endPlayReason: EEndPlayReason)
---@field public GetName fun(self: AActor): string
---@field public GetClass fun(self: AActor): UClass | nil
AActor = {}


--==============================================================================
-- Input Bindings (Example)
--==============================================================================

---@param virtualKeyCode integer
---@return boolean
function IsKeyDown(virtualKeyCode) end

---@type integer
KEY_LEFT = 0x25
---@type integer
KEY_UP = 0x26
---@type integer
KEY_RIGHT = 0x27
---@type integer
KEY_DOWN = 0x28

---@type integer
KEY_A = 0x41
---@type integer
KEY_B = 0x42
---@type integer
KEY_Z = 0x5A


--==============================================================================
-- Placeholders for other potentially bound types
--==============================================================================

---@class UActorComponent : UObject
UActorComponent = {}

---@class USceneComponent : UActorComponent
USceneComponent = {}
