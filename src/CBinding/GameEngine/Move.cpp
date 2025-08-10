//
// Created by PinkySmile on 07/08/2025.
//

#include "Move.h"

PokemonGen1_Move *PokemonGen1_Move_create(
	unsigned char id,
	const char *&name,
	PokemonGen1_Type type,
	PokemonGen1_MoveCategory category,
	unsigned int power,
	unsigned char accuracy,
	unsigned char maxpp,
	PokemonGen1_Move_StatusChangeProb statusChange,
	PokemonGen1_Move_StatsChangeProb *ownerChange, size_t ownerChangeCount,
	PokemonGen1_Move_StatsChangeProb *foeChange, size_t foeChangeCount,
	PokemonGen1_Move_MinMax nbHits,
	PokemonGen1_Move_MinMax nbRuns,
	const char *keepGoingMsg,
	char priority,
	double critChance,
	bool needLoading,
	const char *loadingMsg,
	bool invulnerableDuringLoading,
	bool needRecharge,
	bool (*hitCallback)(
		PokemonGen1_Pokemon *owner,
		PokemonGen1_Pokemon *target,
		unsigned damage,
		bool last,
		void (*logger)(const char *msg, const void *ctx),
		const void *ctx
	),
	const char *hitCallBackDescription,
	bool (*missCallback)(
		PokemonGen1_Pokemon *owner,
		PokemonGen1_Pokemon *target,
		bool last,
		void (*logger)(const char *msg, const void *ctx),
		const void *ctx
	),
	const char *missCallBackDescription
)
{
	return new PokemonGen1_Move(
		id,
		name,
		static_cast<PokemonGen1::Type>(type),
		static_cast<PokemonGen1::MoveCategory>(category),
		power,
		accuracy,
		maxpp,
		statusChange,
		{ownerChange, ownerChange + ownerChangeCount},
		{foeChange, foeChange + foeChangeCount},
		{nbHits.min, nbHits.max},
		{nbRuns.min, nbRuns.max},
		keepGoingMsg,
		priority,
		critChance,
		needLoading,
		loadingMsg,
		invulnerableDuringLoading,
		needRecharge,
		[hitCallback](PokemonGen1::Pokemon &owner, PokemonGen1::Pokemon &target, unsigned damage, bool lastRun, const std::function<void(const std::string &msg)> &logger){
			return hitCallback(&owner, &target, damage, lastRun, [](const char *msg, const void *ctx){
				(*reinterpret_cast<typeof(&logger)>(ctx))(msg);
			}, reinterpret_cast<const void *>(&logger));
		},
		hitCallBackDescription,
		[missCallback](PokemonGen1::Pokemon &owner, PokemonGen1::Pokemon &target, bool last, const std::function<void(const std::string &msg)> &logger){
			return missCallback(&owner, &target, last, [](const char *msg, const void *ctx){
				(*reinterpret_cast<typeof(&logger)>(ctx))(msg);
			}, reinterpret_cast<const void *>(&logger));
		},
		missCallBackDescription
	);
}
PokemonGen1_Move *PokemonGen1_Move_copy(PokemonGen1_Move *object)
{
	return new PokemonGen1_Move(*object);
}
void PokemonGen1_Move_assign_to(PokemonGen1_Move *object, PokemonGen1_Move *target)
{
	(*target) = (*object);
}

PokemonGen1_Move_MinMax PokemonGen1_Move_getNbRuns(const PokemonGen1_Move *object)
{
	const auto &pair = object->getNbRuns();

	return { .min = pair.first, .max = pair.second };
}
PokemonGen1_Move_MinMax PokemonGen1_Move_getNbHits(const PokemonGen1_Move *object)
{
	const auto &pair = object->getNbHits();

	return { .min = pair.first, .max = pair.second };
}
PokemonGen1_Move_StatusChangeProb PokemonGen1_Move_getStatusChange(const PokemonGen1_Move *object)
{
	return object->getStatusChange();
}
const PokemonGen1_Move_StatsChangeProb *PokemonGen1_Move_getOwnerChange(const PokemonGen1_Move *object, size_t *size)
{
	auto &arr = object->getOwnerChange();

	if (size)
		*size = arr.size();
	return arr.data();
}
const PokemonGen1_Move_StatsChangeProb *PokemonGen1_Move_getFoeChange(const PokemonGen1_Move *object, size_t *size)
{
	auto &arr = object->getFoeChange();

	if (size)
		*size = arr.size();
	return arr.data();
}
bool PokemonGen1_Move_needsLoading(const PokemonGen1_Move *object)
{
	return object->needsLoading();
}
bool PokemonGen1_Move_isInvulnerableDuringLoading(const PokemonGen1_Move *object)
{
	return object->isInvulnerableDuringLoading();
}
bool PokemonGen1_Move_needsRecharge(const PokemonGen1_Move *object)
{
	return object->needsRecharge();
}
const char *PokemonGen1_Move_getHitCallBackDescription(const PokemonGen1_Move *object)
{
	return object->getHitCallBackDescription().c_str();
}
const char *PokemonGen1_Move_getMissCallBackDescription(const PokemonGen1_Move *object)
{
	return object->getMissCallBackDescription().c_str();
}
double PokemonGen1_Move_getCritChance(const PokemonGen1_Move *object)
{
	return object->getCritChance();
}
unsigned char PokemonGen1_Move_getAccuracy(const PokemonGen1_Move *object)
{
	return object->getAccuracy();
}
PokemonGen1_MoveCategory PokemonGen1_Move_getCategory(const PokemonGen1_Move *object)
{
	return static_cast<PokemonGen1_MoveCategory>(object->getCategory());
}
bool PokemonGen1_Move_makesInvulnerable(const PokemonGen1_Move *object)
{
	return object->makesInvulnerable();
}
unsigned char PokemonGen1_Move_getMaxPP(const PokemonGen1_Move *object)
{
	return object->getMaxPP();
}
unsigned int PokemonGen1_Move_getPower(const PokemonGen1_Move *object)
{
	return object->getPower();
}
unsigned char PokemonGen1_Move_getPPUp(const PokemonGen1_Move *object)
{
	return object->getPPUp();
}
PokemonGen1_Type PokemonGen1_Move_getType(const PokemonGen1_Move *object)
{
	return static_cast<PokemonGen1_Type>(object->getType());
}
unsigned char PokemonGen1_Move_getPP(const PokemonGen1_Move *object)
{
	return object->getPP();
}
unsigned char PokemonGen1_Move_getID(const PokemonGen1_Move *object)
{
	return object->getID();
}
const char *PokemonGen1_Move_getName(const PokemonGen1_Move *object)
{
	return object->getName().c_str();
}
const char *PokemonGen1_Move_getDescription(const PokemonGen1_Move *object)
{
	return object->getDescription().c_str();
}
char PokemonGen1_Move_getPriority(const PokemonGen1_Move *object)
{
	return object->getPriority();
}
bool PokemonGen1_Move_isFinished(const PokemonGen1_Move *object)
{
	return object->isFinished();
}
unsigned char PokemonGen1_Move_getHitsLeft(const PokemonGen1_Move *object)
{
	return object->getHitsLeft();
}

void PokemonGen1_Move_glitch(PokemonGen1_Move *object)
{
	object->glitch();
}
void PokemonGen1_Move_setPP(PokemonGen1_Move *object, unsigned char pp)
{
	object->setPP(pp);
}
void PokemonGen1_Move_setPPUp(PokemonGen1_Move *object, unsigned char nb)
{
	object->setPPUp(nb);
}
void PokemonGen1_Move_setHitsLeft(PokemonGen1_Move *object, unsigned char nb)
{
	object->setHitsLeft(nb);
}
void PokemonGen1_Move_reset(PokemonGen1_Move *object)
{
	object->reset();
}

bool PokemonGen1_Move_attack(PokemonGen1_Move *object, PokemonGen1_Pokemon *owner, PokemonGen1_Pokemon *target, void (*logger)(const char *msg))
{
	return object->attack(*owner, *target, [logger](const std::string &msg) { logger(msg.c_str()); });
}