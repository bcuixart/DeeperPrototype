#ifndef PHYSICSPROPERTIES_HH
#define PHYSICSPROPERTIES_HH

enum class HitboxType {
	Solid,				// Fully solid, does not move (Tiles)
	SolidMaybeSloped,   // Fully solid, does not move (Sloped tiles that may or may not act as actual slopes)
	SolidBreakable,     // Fully solid, does not move, can be destroyed (Sand tiles)
	SemisolidTotal,     // Only solid from top for all objects, does not move (Semisolid tiles)
	SemisolidPartial,   // Only solid from top for entities, also solid from sides for dynamic objects, does not move (Most decorative objects)
	Entity,             // Only solid from top for other entities, can move and be affected by forces, but not solid (Dogs, cats, etc.)
	Dynamic,            // Can move and be affected by forces, but not solid (Balls, etc.)
	ForceField,         // Cannot move, not solid (Force fields, trampolines, etc.)
	Trigger,            // Cannot move, not solid but can trigger events (Bones, small bushes, etc.)
};

#endif