# Proposed mapping — design only

| Logical slot | Candidate | Existing owner direction |
| --- | --- | --- |
| Body | SK_Body_Underbody + SK_Hands | Body skeletal visuals with shared Manny leader |
| Head | SK_Head | HeadVariant catalog may resolve skeletal follower instead of static proxy |
| Hair | SK_Hair | HairVariant, same owner/leader |
| UpperRobe | SK_UpperRobe | Existing GarmentVariant visual |
| LowerRobe | Six SK_LowerRobe_* | GarmentVariant resolves a small follower array |
| Belt | SK_Belt + SK_Sash | Garment/accessory catalog entry |
| Accessory | SK_Accessory_Jade | Existing AccessoryVariant concept |
| Boots | SK_Boots | Coverage-aware Body/Garment visual entry |

Smallest proposed extension: one existing garment catalog entry gains a list of shared-skeleton visual parts, plus coverage rules for body/hands/boots. UAppearanceComponent owns/registers them and assigns the existing Manny leader pose. Extend that existing catalog/owner only after this geometry passes Gate 1. No new AppearanceRecipe implementation/default change is made here. Current head/hair proxies are static attachments; the document does not pretend skeletal follower slots already exist.

No candidate identity/path enters TypeScript. No custom PlayerV1 gameplay class, special sword authority, second skeleton, duplicate locomotion/action system or production recipe replacement.
