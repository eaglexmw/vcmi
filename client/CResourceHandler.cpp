#include "StdInc.h"
#include "CResourceHandler.h"

#include "CGameInfo.h"
#include "../lib/CFileSystemHandler.h"
#include "UIFramework/ImageClasses.h"

TImagePtr CResourceHandler::getImage(ResourceIdentifier identifier, size_t frame, size_t group, bool fromBegin /*= false*/)
{
	TResourcesMap resources = CGI->filesystemh->getResourcesMap();
	TImagePtr rslt;

	// check if resource is registered
	if(resources.find(identifier) == resources.end())
	{
		tlog2 << "Image with name " << identifier.name << " and type " 
			<< identifier.type << " wasn't found." << std::endl;
		return rslt;
	}

	// get former/origin resource e.g. from lod with fromBegin=true 
	// and get the latest inserted resource with fromBegin=false
	std::list<ResourceLocator> locators = resources.at(identifier);
	ResourceLocator loc;
	if(!fromBegin)
		loc = locators.back();
	else 
		loc = locators.front();

	// check if resource is already loaded
	GraphicsLocator gloc(loc.loader, loc.resourceName);
	gloc.frame = frame;
	gloc.group = group;

	if(images.find(gloc) == images.end())
	{
		// load it
		return loadImage(gloc);
	}

	weak_ptr<IImage> ptr = images.at(gloc);
	if(ptr.expired())
	{
		// load it
		return loadImage(gloc);
	}

	// already loaded, just return resource
	rslt = shared_ptr<IImage>(ptr);
	return rslt;
}

TImagePtr CResourceHandler::getImage(ResourceIdentifier identifier, bool fromBegin /*= false*/)
{
	return getImage(identifier, 0, 0, fromBegin);
}

TImagePtr CResourceHandler::loadImage(const GraphicsLocator & gloc)
{
	ResourceLocator resLoc(gloc.loader, gloc.resourceName);
	TMemoryStreamPtr data = CGI->filesystemh->getResource(resLoc);
	
	// get file info of the locator
	CFileInfo locInfo(gloc.resourceName);

	TImagePtr img;
	if(boost::iequals(locInfo.getExtension(), ".DEF"))
	{
		CDefFile defFile(data);
		img = IImage::createSpriteFromDEF(&defFile, gloc.frame, gloc.group);
	}
	else
	{
		img = IImage::createImageFromFile(data, locInfo.getExtension());
	}

	weak_ptr<IImage> wPtr(img);
	images[gloc] = wPtr;

	return img;
}