#import "MGLStyle.h"

#import "MGLMapView_Private.h"
#import "MGLStyleLayer.h"
#import "MGLFillStyleLayer.h"
#import "MGLLineStyleLayer.h"
#import "MGLCircleStyleLayer.h"
#import "MGLSymbolStyleLayer.h"
#import "MGLRasterStyleLayer.h"
#import "MGLBackgroundStyleLayer.h"

#import "MGLStyle_Private.h"
#import "MGLStyleLayer_Private.h"
#import "MGLSource_Private.h"

#import "NSDate+MGLAdditions.h"

#import "MGLSource.h"
#import "MGLVectorSource.h"
#import "MGLRasterSource.h"
#import "MGLGeoJSONSource.h"

#include <mbgl/util/default_styles.hpp>
#include <mbgl/style/layers/fill_layer.hpp>
#include <mbgl/style/layers/line_layer.hpp>
#include <mbgl/style/layers/symbol_layer.hpp>
#include <mbgl/style/layers/raster_layer.hpp>
#include <mbgl/style/layers/circle_layer.hpp>
#include <mbgl/style/layers/background_layer.hpp>
#include <mbgl/style/sources/geojson_source.hpp>
#include <mbgl/style/sources/vector_source.hpp>
#include <mbgl/style/sources/raster_source.hpp>
#include <mbgl/mbgl.hpp>

@interface MGLStyle ()

@property (nonatomic, readwrite, weak) MGLMapView *mapView;

@end

@implementation MGLStyle

#pragma mark Default style URLs

static_assert(mbgl::util::default_styles::currentVersion == MGLStyleDefaultVersion, "mbgl::util::default_styles::currentVersion and MGLStyleDefaultVersion disagree.");

/// @param name The style’s marketing name, written in lower camelCase.
/// @param fileName The last path component in the style’s URL, excluding the version suffix.
#define MGL_DEFINE_STYLE(name, fileName) \
    static NSURL *MGLStyleURL_##name; \
    + (NSURL *)name##StyleURL { \
        static dispatch_once_t onceToken; \
        dispatch_once(&onceToken, ^{ \
            MGLStyleURL_##name = [self name##StyleURLWithVersion:8]; \
        }); \
        return MGLStyleURL_##name; \
    } \
    \
    + (NSURL *)name##StyleURL##WithVersion:(NSInteger)version { \
        return [NSURL URLWithString:[@"mapbox://styles/mapbox/" #fileName "-v" stringByAppendingFormat:@"%li", (long)version]]; \
    }

MGL_DEFINE_STYLE(streets, streets)
MGL_DEFINE_STYLE(outdoors, outdoors)
MGL_DEFINE_STYLE(light, light)
MGL_DEFINE_STYLE(dark, dark)
MGL_DEFINE_STYLE(satellite, satellite)
MGL_DEFINE_STYLE(satelliteStreets, satellite-streets)

// Make sure all the styles listed in mbgl::util::default_styles::orderedStyles
// are defined above and also declared in MGLStyle.h.
static_assert(6 == mbgl::util::default_styles::numOrderedStyles,
              "mbgl::util::default_styles::orderedStyles and MGLStyle have different numbers of styles.");

// Hybrid has been renamed Satellite Streets, so the last Hybrid version is hard-coded here.
static NSURL *MGLStyleURL_hybrid;
+ (NSURL *)hybridStyleURL {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        MGLStyleURL_hybrid = [NSURL URLWithString:@"mapbox://styles/mapbox/satellite-hybrid-v8"];
    });
    return MGLStyleURL_hybrid;
}

// Emerald is no longer getting new versions as a default style, so the current version is hard-coded here.
static NSURL *MGLStyleURL_emerald;
+ (NSURL *)emeraldStyleURL {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        MGLStyleURL_emerald = [NSURL URLWithString:@"mapbox://styles/mapbox/emerald-v8"];
    });
    return MGLStyleURL_emerald;
}

#pragma mark -

- (instancetype)initWithMapView:(MGLMapView *)mapView {
    if (self = [super init]) {
        _mapView = mapView;
    }
    return self;
}

- (NSString *)name {
    return @(self.mapView.mbglMap->getStyleName().c_str());
}

#pragma mark Sources

- (mbgl::style::Source *)mbglSourceWithIdentifier:(NSString *)identifier
{
    return self.mapView.mbglMap->getSource(identifier.UTF8String);
}

- (MGLSource *)sourceWithIdentifier:(NSString *)identifier
{
    auto s = self.mapView.mbglMap->getSource(identifier.UTF8String);
    
    Class clazz = [self classFromSource:s];
    
    MGLSource *source = [[clazz alloc] init];
    source.sourceIdentifier = identifier;
    source.source = s;
    
    return source;
}

- (Class)classFromSource:(mbgl::style::Source *)source
{
    if (source->is<mbgl::style::VectorSource>()) {
        return MGLVectorSource.class;
    } else if (source->is<mbgl::style::GeoJSONSource>()) {
        return MGLGeoJSONSource.class;
    } else if (source->is<mbgl::style::RasterSource>()) {
        return MGLRasterSource.class;
    }
    
    [NSException raise:@"Source type not handled" format:@""];
    return Nil;
}

- (void)addSource:(MGLSource *)source
{
    self.mapView.mbglMap->addSource([source mbglSource]);
}

- (void)removeSource:(MGLSource *)source
{
    self.mapView.mbglMap->removeSource(source.sourceIdentifier.UTF8String);
}

#pragma mark Style layers

- (NS_MUTABLE_ARRAY_OF(id <MGLStyleLayer>) *)layers
{
    auto layers = self.mapView.mbglMap->getLayers();
    NSMutableArray *styleLayers = [NSMutableArray arrayWithCapacity:layers.size()];
    for (auto &layer : layers) {
        Class clazz = [self classFromLayer:layer];
        id <MGLStyleLayer, MGLStyleLayer_Private> styleLayer = [[clazz alloc] init];
        styleLayer.layerIdentifier = @(layer->getID().c_str());
        styleLayer.layer = layer;
        styleLayer.mapView = self.mapView;
        [styleLayers addObject:styleLayer];
    }
    return styleLayers;
}

- (void)setLayers:(NS_MUTABLE_ARRAY_OF(id <MGLStyleLayer>) *)layers {
    std::vector<mbgl::style::Layer *> rawLayers;
    rawLayers.reserve(layers.count);
    for (id <MGLStyleLayer, MGLStyleLayer_Private> layer in layers) {
        rawLayers.push_back(layer.layer);
    }
    self.mapView.mbglMap->setLayers(rawLayers);
}

- (NSUInteger)countOfLayers
{
    return self.mapView.mbglMap->getLayers().size();
}

- (id <MGLStyleLayer>)objectInLayersAtIndex:(NSUInteger)index
{
    auto layers = self.mapView.mbglMap->getLayers();
    auto layer = layers.at(index);
    Class clazz = [self classFromLayer:layer];
    
    id <MGLStyleLayer, MGLStyleLayer_Private> styleLayer = [[clazz alloc] init];
    styleLayer.layerIdentifier = @(layer->getID().c_str());
    styleLayer.layer = layer;
    styleLayer.mapView = self.mapView;
    return styleLayer;
}

- (void)getLayers:(id <MGLStyleLayer> *)buffer range:(NSRange)inRange
{
    auto layers = self.mapView.mbglMap->getLayers();
    NSUInteger i = 0;
    for (auto layer = *(layers.begin() + inRange.location); i < inRange.length; ++layer, ++i) {
        Class clazz = [self classFromLayer:layer];
        
        id <MGLStyleLayer, MGLStyleLayer_Private> styleLayer = [[clazz alloc] init];
        styleLayer.layerIdentifier = @(layer->getID().c_str());
        styleLayer.layer = layer;
        styleLayer.mapView = self.mapView;
        
        buffer[i] = styleLayer;
    }
}

- (void)insertObject:(id <MGLStyleLayer, MGLStyleLayer_Private>)styleLayer inLayersAtIndex:(NSUInteger)index
{
    auto layers = self.mapView.mbglMap->getLayers();
    if (index == layers.size()) {
        [self addLayer:styleLayer];
    } else {
        auto layerAbove = layers.at(index);
        self.mapView.mbglMap->addLayer(std::unique_ptr<mbgl::style::Layer>(styleLayer.layer), layerAbove->getID());
    }
}

- (void)removeObjectFromLayersAtIndex:(NSUInteger)index
{
    auto layers = self.mapView.mbglMap->getLayers();
    auto layer = layers.at(index);
    self.mapView.mbglMap->removeLayer(layer->getID());
}

- (mbgl::style::Layer *)mbglLayerWithIdentifier:(NSString *)identifier
{
    return self.mapView.mbglMap->getLayer(identifier.UTF8String);
}

- (id <MGLStyleLayer>)layerWithIdentifier:(NSString *)identifier
{
    auto layer = self.mapView.mbglMap->getLayer(identifier.UTF8String);
    
    Class clazz = [self classFromLayer:layer];
    
    id <MGLStyleLayer, MGLStyleLayer_Private> styleLayer = [[clazz alloc] init];
    styleLayer.layerIdentifier = identifier;
    styleLayer.layer = layer;
    styleLayer.mapView = self.mapView;
    
    return styleLayer;
}

- (Class)classFromLayer:(mbgl::style::Layer *)layer
{
    if (layer->is<mbgl::style::FillLayer>()) {
        return MGLFillStyleLayer.class;
    } else if (layer->is<mbgl::style::LineLayer>()) {
        return MGLLineStyleLayer.class;
    } else if (layer->is<mbgl::style::SymbolLayer>()) {
        return MGLSymbolStyleLayer.class;
    } else if (layer->is<mbgl::style::RasterLayer>()) {
        return MGLRasterStyleLayer.class;
    } else if (layer->is<mbgl::style::CircleLayer>()) {
        return MGLCircleStyleLayer.class;
    } else if (layer->is<mbgl::style::BackgroundLayer>()) {
        return MGLBackgroundStyleLayer.class;
    }
    [NSException raise:@"Layer type not handled" format:@""];
    return Nil;
}

- (void)removeLayer:(id <MGLStyleLayer_Private>)styleLayer
{
    [self willChangeValueForKey:@"layers"];
    self.mapView.mbglMap->removeLayer(styleLayer.layer->getID());
    [self didChangeValueForKey:@"layers"];
}

- (void)addLayer:(id <MGLStyleLayer, MGLStyleLayer_Private>)styleLayer
{
    [self willChangeValueForKey:@"layers"];
    self.mapView.mbglMap->addLayer(std::unique_ptr<mbgl::style::Layer>(styleLayer.layer));
    [self didChangeValueForKey:@"layers"];
}

- (void)insertLayer:(id <MGLStyleLayer, MGLStyleLayer_Private>)styleLayer
         belowLayer:(id <MGLStyleLayer, MGLStyleLayer_Private>)belowLayer
{
    [self willChangeValueForKey:@"layers"];
    const mbgl::optional<std::string> belowLayerId{[belowLayer layerIdentifier].UTF8String};
    self.mapView.mbglMap->addLayer(std::unique_ptr<mbgl::style::Layer>(styleLayer.layer), belowLayerId);
    [self didChangeValueForKey:@"layers"];
}

#pragma mark Style classes

- (NS_ARRAY_OF(NSString *) *)styleClasses
{
    const std::vector<std::string> &appliedClasses = self.mapView.mbglMap->getClasses();
    
    NSMutableArray *returnArray = [NSMutableArray arrayWithCapacity:appliedClasses.size()];
    
    for (auto appliedClass : appliedClasses) {
       [returnArray addObject:@(appliedClass.c_str())];
    }
    
    return returnArray;
}

- (void)setStyleClasses:(NS_ARRAY_OF(NSString *) *)appliedClasses
{
    [self setStyleClasses:appliedClasses transitionDuration:0];
}

- (void)setStyleClasses:(NS_ARRAY_OF(NSString *) *)appliedClasses transitionDuration:(NSTimeInterval)transitionDuration
{
    std::vector<std::string> newAppliedClasses;
    
    for (NSString *appliedClass in appliedClasses)
    {
        newAppliedClasses.push_back([appliedClass UTF8String]);
    }
    
    mbgl::style::TransitionOptions transition { { MGLDurationInSeconds(transitionDuration) } };
    self.mapView.mbglMap->setClasses(newAppliedClasses, transition);
}

- (BOOL)hasStyleClass:(NSString *)styleClass
{
    return styleClass && self.mapView.mbglMap->hasClass([styleClass UTF8String]);
}

- (void)addStyleClass:(NSString *)styleClass
{
    if (styleClass)
    {
        self.mapView.mbglMap->addClass([styleClass UTF8String]);
    }
}

- (void)removeStyleClass:(NSString *)styleClass
{
    if (styleClass)
    {
        self.mapView.mbglMap->removeClass([styleClass UTF8String]);
    }
}

@end
