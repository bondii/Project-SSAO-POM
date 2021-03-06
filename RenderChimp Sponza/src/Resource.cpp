#include <cstring>
#include <cstdio>
#include "RenderChimp.h"
#include "Resource.h"
#include "Misc.h"

/*---------------------------------------------------------------------------*/

Resource::Resource(
		const char				*nidentifier,
		u32						purge_level,
		ResourceType			rtype
	)
{
	if (!nidentifier || *nidentifier == 0)
		REPORT_WARNING("No resource identifier given");

	broken = false;

	next = 0;
	identifier = duplicateString(nidentifier);
	hash = calculateStringHash(identifier);

	purgeLevel = purge_level;

	type = rtype;
}

/*---------------------------------------------------------------------------*/

Resource::~Resource() {

	if (identifier)
		free(identifier);
}

/*---------------------------------------------------------------------------*/

bool Resource::isEqual(
		Resource		*res
	)
{
	return hash == res->hash &&
		strcmp(identifier, res->identifier) == 0;
}

/*---------------------------------------------------------------------------*/

Resource *Resource::instantiate()
{
	n_refs++;

	return this;
}

/*---------------------------------------------------------------------------*/

ResourceType Resource::getType() const
{
	return type;
}

/*---------------------------------------------------------------------------*/

void Resource::setPurgeLevel(
		u32				purge_level
	)
{
	purgeLevel = purge_level;
}

/*---------------------------------------------------------------------------*/

void Resource::setMaxPurgeLevel(
		u32				purge_level
	)
{
	purgeLevel = purge_level > purgeLevel ? purge_level : purgeLevel;
}

/*---------------------------------------------------------------------------*/

u32 Resource::getPurgeLevel() const
{
	return purgeLevel;
}

/*---------------------------------------------------------------------------*/

u32 Resource::getReferenceCount() const
{
	return n_refs;
}

/*---------------------------------------------------------------------------*/

const char *Resource::getIdentifier() const
{
	return identifier;
}

/*---------------------------------------------------------------------------*/

char *Resource::getIdentifierStringCopy()
{
	return duplicateString(identifier);
}

/*---------------------------------------------------------------------------*/

template <> ResourceType Resource::getResourceTypeOf<VertexArray>()   {return RESOURCE_VERTEX_ARRAY;}
template <> ResourceType Resource::getResourceTypeOf<Texture>()       {return RESOURCE_TEXTURE;}
template <> ResourceType Resource::getResourceTypeOf<RenderTarget>()  {return RESOURCE_RENDER_TARGET;}
template <> ResourceType Resource::getResourceTypeOf<CubeMap>()       {return RESOURCE_CUBEMAP;}
template <> ResourceType Resource::getResourceTypeOf<ShaderProgram>() {return RESOURCE_SHADER_PROGRAM;}

/*---------------------------------------------------------------------------*/


