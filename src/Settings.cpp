#include "Settings.h"

Settings::Settings()
{
    juce::PropertiesFile::Options opts;
    opts.applicationName = "TellMeKey";
    opts.filenameSuffix = "xml";
    opts.folderName = "TellMeKey";
    opts.osxLibrarySubFolder = "Application Support";
    opts.storageFormat = juce::PropertiesFile::storeAsXML;

    props = std::make_unique<juce::PropertiesFile> (opts);
}

juce::String Settings::getCustomKeymapPath() const
{
    return props->getValue ("customKeymapPath");
}

void Settings::setCustomKeymapPath (const juce::String& path)
{
    props->setValue ("customKeymapPath", path);
    props->saveIfNeeded();
}
