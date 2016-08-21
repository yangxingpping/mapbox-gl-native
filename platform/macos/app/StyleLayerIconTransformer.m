#import "StyleLayerIconTransformer.h"

#import <Mapbox/Mapbox.h>

@implementation StyleLayerIconTransformer

+ (Class)transformedValueClass {
    return [NSString class];
}

+ (BOOL)allowsReverseTransformation {
    return NO;
}

- (id)transformedValue:(id <MGLStyleLayer>)layer {
    if (![layer conformsToProtocol:@protocol(MGLStyleLayer)]) {
        return nil;
    }
    
    if ([layer isKindOfClass:[MGLBackgroundStyleLayer class]]) {
        return [NSImage imageNamed:@"background"];
    }
    if ([layer isKindOfClass:[MGLCircleStyleLayer class]]) {
        return [NSImage imageNamed:@"circle"];
    }
    if ([layer isKindOfClass:[MGLFillStyleLayer class]]) {
        return [NSImage imageNamed:@"fill"];
    }
    if ([layer isKindOfClass:[MGLLineStyleLayer class]]) {
        return [NSImage imageNamed:@"NSListViewTemplate"];
    }
    if ([layer isKindOfClass:[MGLRasterStyleLayer class]]) {
        return [[NSWorkspace sharedWorkspace] iconForFileType:@"jpg"];
    }
    if ([layer isKindOfClass:[MGLSymbolStyleLayer class]]) {
        return [NSImage imageNamed:@"symbol"];
    }
    
    return nil;
}

@end
