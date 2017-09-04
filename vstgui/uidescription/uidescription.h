// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __uidescription__
#define __uidescription__

#include "../lib/idependency.h"
#include "iuidescription.h"
#include "uidescriptionfwd.h"
#include "xmlparser.h"
#include <list>
#include <string>
#include <memory>

namespace VSTGUI {

class UINode;

//-----------------------------------------------------------------------------
/// @brief XML description parser and view creator
/// @ingroup new_in_4_0
//-----------------------------------------------------------------------------
class UIDescription : public NonAtomicReferenceCounted, public IUIDescription, public Xml::IHandler, public IDependency
{
public:
	UIDescription (const CResourceDescription& xmlFile, IViewFactory* viewFactory = nullptr);
	UIDescription (Xml::IContentProvider* xmlContentProvider, IViewFactory* viewFactory = nullptr);
	~UIDescription () noexcept override;

	virtual bool parse ();

	enum SaveFlags {
		kWriteWindowsResourceFile	= 1 << 0,
		kWriteImagesIntoXMLFile		= 1 << 1
	};

	virtual bool save (UTF8StringPtr filename, int32_t flags = kWriteWindowsResourceFile);
	virtual bool saveWindowsRCFile (UTF8StringPtr filename);

	bool storeViews (const std::list<CView*>& views, OutputStream& stream, UIAttributes* customData = nullptr) const;
	bool restoreViews (InputStream& stream, std::list<SharedPointer<CView> >& views, UIAttributes** customData = nullptr);

	UTF8StringPtr getFilePath () const;
	void setFilePath (UTF8StringPtr path);
	
	void setSharedResources (const SharedPointer<UIDescription>& resources);
	const SharedPointer<UIDescription>& getSharedResources () const;
	
	const UIAttributes* getViewAttributes (UTF8StringPtr name) const;

	void setController (IController* controller) const;

	CView* createView (UTF8StringPtr name, IController* controller) const override;
	CBitmap* getBitmap (UTF8StringPtr name) const override;
	CFontRef getFont (UTF8StringPtr name) const override;
	bool getColor (UTF8StringPtr name, CColor& color) const override;
	CGradient* getGradient (UTF8StringPtr name) const override;
	int32_t getTagForName (UTF8StringPtr name) const override;
	IControlListener* getControlListener (UTF8StringPtr name) const override;
	IController* getController () const override;
	const IViewFactory* getViewFactory () const override;
	
	UTF8StringPtr lookupColorName (const CColor& color) const override;
	UTF8StringPtr lookupFontName (const CFontRef font) const override;
	UTF8StringPtr lookupBitmapName (const CBitmap* bitmap) const override;
	UTF8StringPtr lookupGradientName (const CGradient* gradient) const override;
	UTF8StringPtr lookupControlTagName (const int32_t tag) const override;

	bool getVariable (UTF8StringPtr name, double& value) const override;
	bool getVariable (UTF8StringPtr name, std::string& value) const override;

	void collectTemplateViewNames (std::list<const std::string*>& names) const override;
	void collectColorNames (std::list<const std::string*>& names) const override;
	void collectFontNames (std::list<const std::string*>& names) const override;
	void collectBitmapNames (std::list<const std::string*>& names) const override;
	void collectGradientNames (std::list<const std::string*>& names) const override;
	void collectControlTagNames (std::list<const std::string*>& names) const override;
	
	void changeColorName (UTF8StringPtr oldName, UTF8StringPtr newName);
	void changeTagName (UTF8StringPtr oldName, UTF8StringPtr newName);
	void changeFontName (UTF8StringPtr oldName, UTF8StringPtr newName);
	void changeBitmapName (UTF8StringPtr oldName, UTF8StringPtr newName);
	void changeGradientName (UTF8StringPtr oldName, UTF8StringPtr newName);

	void changeColor (UTF8StringPtr name, const CColor& newColor);
	void changeFont (UTF8StringPtr name, CFontRef newFont);
	void changeGradient (UTF8StringPtr name, CGradient* newGradient);
	void changeBitmap (UTF8StringPtr name, UTF8StringPtr newName, const CRect* nineparttiledOffset = nullptr);

	void changeBitmapFilters (UTF8StringPtr bitmapName, const std::list<SharedPointer<UIAttributes> >& filters);
	void collectBitmapFilters (UTF8StringPtr bitmapName, std::list<SharedPointer<UIAttributes> >& filters) const;
	
	void removeColor (UTF8StringPtr name);
	void removeTag (UTF8StringPtr name);
	void removeFont (UTF8StringPtr name);
	void removeBitmap (UTF8StringPtr name);
	void removeGradient (UTF8StringPtr name);

	void changeAlternativeFontNames (UTF8StringPtr name, UTF8StringPtr alternativeFonts);
	bool getAlternativeFontNames (UTF8StringPtr name, std::string& alternativeFonts) const;

	bool hasColorName (UTF8StringPtr name) const;
	bool hasTagName (UTF8StringPtr name) const;
	bool hasFontName (UTF8StringPtr name) const;
	bool hasBitmapName (UTF8StringPtr name) const;
	bool hasGradientName (UTF8StringPtr name) const;

	void updateViewDescription (UTF8StringPtr name, CView* view);
	bool getTemplateNameFromView (CView* view, std::string& templateName) const;
	bool addNewTemplate (UTF8StringPtr name, UIAttributes* attr); // owns attributes
	bool removeTemplate (UTF8StringPtr name);
	bool changeTemplateName (UTF8StringPtr name, UTF8StringPtr newName);
	bool duplicateTemplate (UTF8StringPtr name, UTF8StringPtr duplicateName);

	bool setCustomAttributes (UTF8StringPtr name, UIAttributes* attr); //owns attributes
	UIAttributes* getCustomAttributes (UTF8StringPtr name) const;
	UIAttributes* getCustomAttributes (UTF8StringPtr name, bool create);

	bool getControlTagString (UTF8StringPtr tagName, std::string& tagString) const;
	bool changeControlTagString  (UTF8StringPtr tagName, const std::string& newTagString, bool create = false);

	bool calculateStringValue (UTF8StringPtr str, double& result) const;

	void registerListener (UIDescriptionListener* listener);
	void unregisterListener (UIDescriptionListener* listener);

	void setBitmapCreator (IBitmapCreator* bitmapCreator);

	struct FocusDrawing
	{
		bool enabled {false};
		CCoord width {1};
		UTF8String colorName;
	};
	FocusDrawing getFocusDrawingSettings () const;
	void setFocusDrawingSettings (const FocusDrawing& fd);
	
	void freePlatformResources ();

	static bool parseColor (const std::string& colorString, CColor& color);
	static CViewAttributeID kTemplateNameAttributeID;
	
	static IdStringPtr kMessageTagChanged;
	static IdStringPtr kMessageColorChanged;
	static IdStringPtr kMessageFontChanged;
	static IdStringPtr kMessageBitmapChanged;
	static IdStringPtr kMessageTemplateChanged;
	static IdStringPtr kMessageGradientChanged;
	static IdStringPtr kMessageBeforeSave;
protected:
	void addDefaultNodes ();

	bool saveToStream (OutputStream& stream, int32_t flags);

	bool parsed () const;
	void setXmlContentProvider (Xml::IContentProvider* provider);

	const CResourceDescription& getXmlFile () const;
private:
	// Xml::IHandler
	void startXmlElement (Xml::Parser* parser, IdStringPtr elementName, UTF8StringPtr* elementAttributes) override;
	void endXmlElement (Xml::Parser* parser, IdStringPtr name) override;
	void xmlCharData (Xml::Parser* parser, const int8_t* data, int32_t length) override;
	void xmlComment (Xml::Parser* parser, IdStringPtr comment) override;
	
	CView* createViewFromNode (UINode* node) const;
	UINode* getBaseNode (UTF8StringPtr name) const;
	UINode* findChildNodeByNameAttribute (UINode* node, UTF8StringPtr nameAttribute) const;
	UINode* findNodeForView (CView* view) const;
	bool updateAttributesForView (UINode* node, CView* view, bool deep = true);
	void removeNode (UTF8StringPtr name, IdStringPtr mainNodeName, IdStringPtr changeMsg);
	template<typename NodeType, typename ObjType, typename CompareFunction> UTF8StringPtr lookupName (const ObjType& obj, IdStringPtr mainNodeName, CompareFunction compare) const;
	template<typename NodeType> void changeNodeName (UTF8StringPtr oldName, UTF8StringPtr newName, IdStringPtr mainNodeName, IdStringPtr changeMsg);
	template<typename NodeType> void collectNamesFromNode (IdStringPtr mainNodeName, std::list<const std::string*>& names) const;
	
	struct Impl;
	std::unique_ptr<Impl> impl;
};
 
//-----------------------------------------------------------------------------
class IBitmapCreator
{
public:
	virtual ~IBitmapCreator () noexcept = default;
	
	virtual SharedPointer<IPlatformBitmap> createBitmap (const UIAttributes& attributes) = 0;
};

} // namespace

#endif
