#import "MGLFeature_Private.h"

#import "MGLPointAnnotation.h"
#import "MGLPolyline.h"
#import "MGLPolygon.h"
#import "MGLValueEvaluator.h"

#import "MGLMultiPoint_Private.h"
#import "MGLPolyline+MGLAdditions.h"
#import "MGLPolygon+MGLAdditions.h"
#import <mbgl/util/geometry.hpp>

@interface MGLFeatureHelper : NSObject

+ (mbgl::Value)convertedValueWithValue:(id)value;

@end

// Multiple numeric types (uint64_t, int64_t, double) are present in order to support
// the widest possible range of JSON numbers, which do not have a maximum range.
// Implementations that produce `value`s should use that order for type preference,
// using uint64_t for positive integers, int64_t for negative integers, and double
// for non-integers and integers outside the range of 64 bits.
//using value_base = mapbox::util::variant<null_value_t, bool, uint64_t, int64_t, double, std::string,
//mapbox::util::recursive_wrapper<std::vector<value>>,
//mapbox::util::recursive_wrapper<std::unordered_map<std::string, value>>>;
@implementation MGLFeatureHelper

+ (mbgl::Value)convertedValueWithValue:(id)value {
    if ([value isKindOfClass:NSString.class]) {
        return { std::string([(NSString *)value UTF8String]) };
    } else if ([value isKindOfClass:NSNumber.class]) {
        NSNumber *number = (NSNumber *)value;
        if ((strcmp([number objCType], @encode(char)) == 0) ||
            (strcmp([number objCType], @encode(BOOL)) == 0)) {
            // char: 32-bit boolean
            // BOOL: 64-bit boolean
            return { (bool)number.boolValue };
        } else if (strcmp([number objCType], @encode(double)) == 0) {
            // Double values on all platforms are interpreted precisely.
            return { (double)number.doubleValue };
        } else if (strcmp([number objCType], @encode(float)) == 0) {
            // Float values when taken as double introduce precision problems,
            // so warn the user to avoid them. This would require them to
            // explicitly use -[NSNumber numberWithFloat:] arguments anyway.
            // We still do this conversion in order to provide a valid value.
            static dispatch_once_t onceToken;
            dispatch_once(&onceToken, ^{
                NSLog(@"One-time warning: Float values are converted to double and can introduce imprecision; please use double values explicitly in predicate arguments.");
            });
            return { (double)number.doubleValue };
        } else if ([number compare:@(0)] == NSOrderedDescending ||
                   [number compare:@(0)] == NSOrderedSame) {
            // Positive integer or zero; use uint64_t per mbgl::Value definition.
            // We use unsigned long long here to avoid any truncation.
            return { (uint64_t)number.unsignedLongLongValue };
        } else if ([number compare:@(0)] == NSOrderedAscending) {
            // Negative integer; use int64_t per mbgl::Value definition.
            // We use long long here to avoid any truncation.
            return { (int64_t)number.longLongValue };
        }
    }
    [NSException raise:@"Value not handled"
                format:@"Can’t convert %s:%@ to mbgl::Value", [value objCType], value];
    return { };
}

@end

@interface MGLPointFeature () <MGLFeaturePrivate>
@end

@implementation MGLPointFeature

@synthesize identifier;
@synthesize attributes;

- (id)attributeForKey:(NSString *)key {
    return self.attributes[key];
}

- (NS_DICTIONARY_OF(NSString *, id) *)featureDictionary {
    
    return @{@"type":@"Feature",
             @"properties":(self.attributes) ? self.attributes : @{},
                     @"geometry":@{
                             @"type":@"Point",
                             @"coordinates":@[@(self.coordinate.longitude), @(self.coordinate.latitude)]
                             }
            };
}

@end

@interface MGLPolylineFeature () <MGLFeaturePrivate>
@end

@implementation MGLPolylineFeature

@synthesize identifier;
@synthesize attributes;

- (id)attributeForKey:(NSString *)key {
    return self.attributes[key];
}

- (NS_DICTIONARY_OF(NSString *, id) *)featureDictionary {
    return @{@"type":@"Feature",
             @"properties":(self.attributes) ? self.attributes : @{},
             @"geometry":@{
                     @"type":@"LineString",
                     @"coordinates":self.mgl_coordinates
                     }
             };;
}

@end

@interface MGLPolygonFeature () <MGLFeaturePrivate>
@end

@implementation MGLPolygonFeature

@synthesize identifier;
@synthesize attributes;

- (id)attributeForKey:(NSString *)key {
    return self.attributes[key];
}

- (NS_DICTIONARY_OF(NSString *, id) *)featureDictionary {
    return @{@"type":@"Feature",
             @"properties":(self.attributes) ? self.attributes : @{},
             @"geometry":@{
                    @"type":@"Polygon",
                    @"coordinates":self.mgl_coordinates
                    }
            };
}

- (mbgl::Feature)mbglFeature {
    auto feature = [self featureObject];
    mbgl::PropertyMap propertyMap;
    for (NSString *key in self.attributes.allKeys) {
        id value = self.attributes[key];
    #warning temporary debug logging
        NSLog(@"================> %@, %@", key, value);
        propertyMap[[key cStringUsingEncoding:NSUTF8StringEncoding]] = [MGLFeatureHelper convertedValueWithValue:value];
    }
    feature.properties = propertyMap;
    return feature;
}

@end

@interface MGLMultiPointFeature () <MGLFeaturePrivate>
@end

@implementation MGLMultiPointFeature

@synthesize identifier;
@synthesize attributes;

- (id)attributeForKey:(NSString *)key {
    return self.attributes[key];
}

- (NS_DICTIONARY_OF(NSString *, id) *)featureDictionary {
    NSMutableArray *coordinates = [NSMutableArray array];
    
    for (NSUInteger index = 0; index < self.pointCount; index++) {
        CLLocationCoordinate2D coordinate = self.coordinates[index];
        [coordinates addObject:@[@(coordinate.longitude), @(coordinate.latitude)]];
    }
    
    return @{@"type":@"Feature",
             @"properties":(self.attributes) ? self.attributes : @{},
             @"geometry":@{
                     @"type":@"Multipoint",
                     @"coordinates":coordinates
                     }
             };
}

@end

@interface MGLMultiPolylineFeature () <MGLFeaturePrivate>
@end

@implementation MGLMultiPolylineFeature

@synthesize identifier;
@synthesize attributes;

- (id)attributeForKey:(NSString *)key {
    return self.attributes[key];
}

- (NS_DICTIONARY_OF(NSString *, id) *)featureDictionary {
    NSMutableArray *coordinates = [NSMutableArray array];
    for (MGLPolylineFeature *feature in self.polylines) {
        [coordinates addObject:feature.mgl_coordinates];
    }
    
    return @{@"type":@"Feature",
             @"properties":(self.attributes) ? self.attributes : @{},
             @"geometry":@{
                     @"type":@"MultiLineString",
                     @"coordinates":coordinates
                     }
             };
}

@end

@interface MGLMultiPolygonFeature () <MGLFeaturePrivate>
@end

@implementation MGLMultiPolygonFeature

@synthesize identifier;
@synthesize attributes;

- (id)attributeForKey:(NSString *)key {
    return self.attributes[key];
}

- (NS_DICTIONARY_OF(NSString *, id) *)featureDictionary {
    NSMutableArray *coordinates = [NSMutableArray array];
    for (MGLPolygonFeature *feature in self.polygons) {
        [coordinates addObject:feature.mgl_coordinates];
    }
    
    return @{@"type":@"Feature",
             @"properties":(self.attributes) ? self.attributes : @{},
             @"geometry":@{
                     @"type":@"MultiPolygon",
                     @"coordinates":coordinates
                     }
             };
}

@end

@interface MGLShapeCollectionFeature () <MGLFeaturePrivate>
@end

@implementation MGLShapeCollectionFeature

@synthesize identifier;
@synthesize attributes;

@dynamic shapes;

+ (instancetype)shapeCollectionWithShapes:(NS_ARRAY_OF(MGLShape<MGLFeature> *) *)shapes {
    return [super shapeCollectionWithShapes:shapes];
}

- (id)attributeForKey:(NSString *)key {
    return self.attributes[key];
}

- (NS_DICTIONARY_OF(NSString *, id) *)featureDictionary {
    
    return @{@"type":@"Feature",
             @"properties":(self.attributes) ? self.attributes : @{},
             @"geometry":@{
                     @"type":@"GeometryCollection",
                     @"geometries":[self geometryCollection:self.shapes]
                     }
             };
}

- (NS_MUTABLE_ARRAY_OF(NS_DICTIONARY_OF(NSString *, id) *) *)geometryCollection:(NS_ARRAY_OF(MGLShape<MGLFeature> *) *)shapes {
    NSMutableArray *geometries = [NSMutableArray array];
    
    for (MGLShape<MGLFeature> *shape in shapes) {
        if ([shape isKindOfClass:[MGLShapeCollectionFeature class]]) {
            [geometries addObject:@{@"type":@"GeometryCollection",
                                    @"geometries":[self geometryCollection:((MGLShapeCollectionFeature *)shape).shapes]}];
        } else {
            NSDictionary *geometry = shape.featureDictionary[@"geometry"];
            [geometries addObject:@{@"type":geometry[@"type"],
                                @"coordinates":geometry[@"coordinates"] }];
        }
    }
    
    return geometries;
}
@end



/**
 Transforms an `mbgl::geometry::geometry` type into an instance of the
 corresponding Objective-C geometry class.
 */
template <typename T>
class GeometryEvaluator {
public:
    MGLShape <MGLFeaturePrivate> * operator()(const mbgl::Point<T> &geometry) const {
        MGLPointFeature *feature = [[MGLPointFeature alloc] init];
        feature.coordinate = toLocationCoordinate2D(geometry);
        return feature;
    }
    
    MGLShape <MGLFeaturePrivate> * operator()(const mbgl::LineString<T> &geometry) const {
        std::vector<CLLocationCoordinate2D> coordinates = toLocationCoordinates2D(geometry);
        return [MGLPolylineFeature polylineWithCoordinates:&coordinates[0] count:coordinates.size()];
    }
    
    MGLShape <MGLFeaturePrivate> * operator()(const mbgl::Polygon<T> &geometry) const {
        return toShape<MGLPolygonFeature>(geometry);
    }
    
    MGLShape <MGLFeaturePrivate> * operator()(const mbgl::MultiPoint<T> &geometry) const {
        std::vector<CLLocationCoordinate2D> coordinates = toLocationCoordinates2D(geometry);
        return [[MGLMultiPointFeature alloc] initWithCoordinates:&coordinates[0] count:coordinates.size()];
    }
    
    MGLShape <MGLFeaturePrivate> * operator()(const mbgl::MultiLineString<T> &geometry) const {
        NSMutableArray *polylines = [NSMutableArray arrayWithCapacity:geometry.size()];
        for (auto &lineString : geometry) {
            std::vector<CLLocationCoordinate2D> coordinates = toLocationCoordinates2D(lineString);
            MGLPolyline *polyline = [MGLPolyline polylineWithCoordinates:&coordinates[0] count:coordinates.size()];
            [polylines addObject:polyline];
        }
        
        return [MGLMultiPolylineFeature multiPolylineWithPolylines:polylines];
    }
    
    MGLShape <MGLFeaturePrivate> * operator()(const mbgl::MultiPolygon<T> &geometry) const {
        NSMutableArray *polygons = [NSMutableArray arrayWithCapacity:geometry.size()];
        for (auto &polygon : geometry) {
            [polygons addObject:toShape(polygon)];
        }
        
        return [MGLMultiPolygonFeature multiPolygonWithPolygons:polygons];
    }
    
    MGLShape <MGLFeaturePrivate> * operator()(const mapbox::geometry::geometry_collection<T> &collection) const {
        NSMutableArray *shapes = [NSMutableArray arrayWithCapacity:collection.size()];
        for (auto &geometry : collection) {
            // This is very much like the transformation that happens in MGLFeaturesFromMBGLFeatures(), but these are raw geometries with no associated feature IDs or attributes.
            MGLShape <MGLFeaturePrivate> *shape = mapbox::geometry::geometry<T>::visit(geometry, *this);
            [shapes addObject:shape];
        }
        return [MGLShapeCollectionFeature shapeCollectionWithShapes:shapes];
    }
    
private:
    static CLLocationCoordinate2D toLocationCoordinate2D(const mbgl::Point<T> &point) {
        return CLLocationCoordinate2DMake(point.y, point.x);
    }
    
    static std::vector<CLLocationCoordinate2D> toLocationCoordinates2D(const std::vector<mbgl::Point<T>> &points) {
        std::vector<CLLocationCoordinate2D> coordinates;
        coordinates.reserve(points.size());
        std::transform(points.begin(), points.end(), std::back_inserter(coordinates), toLocationCoordinate2D);
        return coordinates;
    }
    
    template<typename U = MGLPolygon>
    static U *toShape(const mbgl::Polygon<T> &geometry) {
        auto &linearRing = geometry.front();
        std::vector<CLLocationCoordinate2D> coordinates = toLocationCoordinates2D(linearRing);
        NSMutableArray *innerPolygons;
        if (geometry.size() > 1) {
            innerPolygons = [NSMutableArray arrayWithCapacity:geometry.size() - 1];
            for (auto iter = geometry.begin() + 1; iter != geometry.end(); iter++) {
                auto &innerRing = *iter;
                std::vector<CLLocationCoordinate2D> coordinates = toLocationCoordinates2D(innerRing);
                MGLPolygon *innerPolygon = [MGLPolygon polygonWithCoordinates:&coordinates[0] count:coordinates.size()];
                [innerPolygons addObject:innerPolygon];
            }
        }
        
        return [U polygonWithCoordinates:&coordinates[0] count:coordinates.size() interiorPolygons:innerPolygons];
    }
};

NS_ARRAY_OF(MGLShape <MGLFeature> *) *MGLFeaturesFromMBGLFeatures(const std::vector<mbgl::Feature> &features) {
    NSMutableArray *shapes = [NSMutableArray arrayWithCapacity:features.size()];
    for (const auto &feature : features) {
        NSMutableDictionary *attributes = [NSMutableDictionary dictionaryWithCapacity:feature.properties.size()];
        for (auto &pair : feature.properties) {
            auto &value = pair.second;
            ValueEvaluator evaluator;
            attributes[@(pair.first.c_str())] = mbgl::Value::visit(value, evaluator);
        }
        
        GeometryEvaluator<double> evaluator;
        MGLShape <MGLFeaturePrivate> *shape = mapbox::geometry::geometry<double>::visit(feature.geometry, evaluator);
        if (feature.id) {
            shape.identifier = mbgl::FeatureIdentifier::visit(*feature.id, ValueEvaluator());
        }
        shape.attributes = attributes;
        [shapes addObject:shape];
    }
    return shapes;
}
