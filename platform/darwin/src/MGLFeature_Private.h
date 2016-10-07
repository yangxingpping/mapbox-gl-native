#import "MGLFeature.h"
#import "MGLShape.h"

#import <mbgl/util/geo.hpp>
#import <mbgl/util/feature.hpp>

NS_ASSUME_NONNULL_BEGIN

/**
 Returns an array of `MGLFeature` objects converted from the given vector of
 vector tile features.
 */
NS_ARRAY_OF(MGLShape <MGLFeature> *) *MGLFeaturesFromMBGLFeatures(const std::vector<mbgl::Feature> &features);

@protocol MGLFeaturePrivate <MGLFeature>

@property (nonatomic, copy, nullable, readwrite) id identifier;
@property (nonatomic, copy, readwrite) NS_DICTIONARY_OF(NSString *, id) *attributes;

- (mbgl::Feature)mbglFeature;

@end

NS_ASSUME_NONNULL_END
