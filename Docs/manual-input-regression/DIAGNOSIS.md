# Manual input and action replay regression

Starting revision: `ee7a15eca975404c4ff196ca0a0650d3a4bbac31`, clean checkout, branch `feat/ue58-playfeel-correction`. Fix branch: `fix/ue58-manual-input-actions`. Visual Gate 2 remains out of scope.

## Real input reproduction before modifying code

The existing StillwaterValley project was opened in UE 5.8.3 without any bridge arguments, matching ordinary Editor PIE. After Windows was unlocked, computer-use delivered actual E, M and mouse LMB inputs to the possessed PIE viewport. No adapter method or injected InputAction was used for this reproduction.

E changed the small HUD feedback from “Cultivation simulator not connected” to “No controlled simulation actor bound”; M reached the same result. This proves the physical key mapping/callback exists. The first missing command step is the absence of external bridge configuration and an authoritative controlled-actor snapshot. There is no inbox write, reservation or receipt in this unconfigured flow. It is inaccurate to call the mapping absent: the feedback is too subtle and does not explain the required launcher. The screenshot-visible response and baseline Editor log are retained in the conversation/evidence.

First physical LMB starts Sword. After 1.4 seconds the enum returns to Locomotion. A second physical LMB again enters Sword, but does not replay the full slash. The baseline log records first Sword at 20:12:33.961 UTC, Locomotion at 20:12:35.343, and second Sword at 20:12:48.003. The second click was therefore not rejected by a permanently active action enum.

The proxy only reset accumulated sequence time when the asset pointer changed. Returning to locomotion intentionally retained ActionClip for blend-out. The next sword activation uses the same asset, so the sequence remains at its old end time. Enum transitions alone cannot catch this defect. The same issue can affect repeated meditation clips as well.

## Mapping audit and comparison

DefaultPlayerInputClass and DefaultInputComponentClass use Enhanced Input. The PlayerController installs the command context at priority 10 with separate Boolean E/M actions, both bound once to Started. The pawn's movement/sword context is priority 0. LMB is a pawn Started binding. Normal PIE uses GameOnly input, a possessed pawn and captured viewport. There is no ClearAllMappings, command-context removal or new play-feel context. The developer-preview flag deliberately selects the old cosmetic-only bindings; it is not used for the production/authority tests.

The PlayerController, command adapter and E/M mappings are unchanged between `97cdbe3` and `ee7a15e`. The same-asset reset defect also existed in the earlier proxy. The correction commit exposed it more clearly with one persistent action branch and a readable authored slash; blindly reverting that commit would not repair the lifecycle.

The old round-trip test enabled transport and injected the Cultivate InputAction into a prepared external worker fixture. That verifies the action callback and authoritative protocol, but bypasses physical key mapping, ordinary unconfigured PIE and clear configuration feedback. The correction sequence called SwordAction directly and did not assert the second attack's sequence restart. Both coverage gaps are now addressed separately.

## Focused changes

- E/M still use the existing command context and one binding per action. They now log Started/Completed with action identity, count received attempts and display prominent CONFIG UNAVAILABLE, WORKER UNAVAILABLE or SIMULATION NOT READY states in the existing HUD. Missing configuration points to Tools/RunCultivationCommandDemo.ps1. No local success/meditation shortcut is introduced.
- A configured bridge with no worker.lock is reported as unavailable rather than silently writing to an unattended inbox. The lock check detects absence; it is not a claim that a stale lock guarantees a live worker. Existing sent/accepted/rejected/completed feedback remains authoritative.
- Every accepted non-locomotion phase increments one ActionRevision. The native proxy restarts sequence time when that revision changes, even if the asset is identical. Locomotion retains the last clip for smooth blend-out. The canonical phase gate remains Ready → Sword → recovery → Ready. Clicks while active are ignored, with no queue or combo.
- Diagnostic raw-key logging in PlayerController delegates to Super unchanged. It records E/M/LMB press/release, possession and adapter presence. Sword request logs phase/ready; sequence restart logs revision/asset. No simulation dependency enters the AnimInstance.

Grip transform, sword assets/path, upper-body action mask, landing intervals/timing, steering, camera and foot IK are not changed by this fix.
