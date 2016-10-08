#import "MGLStyleValue_Private.h"

@implementation MGLStyleValue

+ (instancetype)valueWithRawValue:(id)rawValue {
    return [MGLStyleConstantValue valueWithRawValue:rawValue];
}

+ (instancetype)valueWithBase:(CGFloat)base stops:(NSDictionary *)stops {
    return [MGLStyleFunction functionWithBase:base stops:stops];
}

+ (instancetype)valueWithStops:(NSDictionary *)stops {
    return [MGLStyleFunction functionWithStops:stops];
}

- (id)rawValueAtZoomLevel:(double)zoomLevel {
    return nil;
}

@end

@implementation MGLStyleConstantValue

+ (instancetype)valueWithRawValue:(id)rawValue {
    return [[self alloc] initWithRawValue:rawValue];
}

- (instancetype)initWithRawValue:(id)rawValue {
    if (self = [super init]) {
        _rawValue = rawValue;
    }
    return self;
}

- (id)rawValueAtZoomLevel:(double)zoomLevel {
    return self.rawValue;
}

- (NSString *)description {
    return [self.rawValue description];
}

- (BOOL)isEqual:(MGLStyleConstantValue *)other {
    return [other isKindOfClass:[self class]] && [other.rawValue isEqual:self.rawValue];
}

- (NSUInteger)hash {
    return [self.rawValue hash];
}

@end

@implementation MGLStyleFunction

+ (instancetype)functionWithBase:(CGFloat)base stops:(NSDictionary *)stops {
    return [[self alloc] initWithBase:base stops:stops];
}

+ (instancetype)functionWithStops:(NSDictionary *)stops {
    return [[self alloc] initWithBase:1 stops:stops];
}

- (instancetype)init {
    if (self = [super init]) {
        _base = 1;
    }
    return self;
}

- (instancetype)initWithBase:(CGFloat)base stops:(NSDictionary *)stops {
    if (self = [self init]) {
        _base = base;
        _stops = stops;
    }
    return self;
}

- (id)rawValueAtZoomLevel:(double)zoomLevel {
#warning rawValueAtZoomLevel for zoom function
    return nil;
}

- (NSString *)description {
    return [NSString stringWithFormat:@"<%@: %p, base = %f; stops = %@>",
            NSStringFromClass([self class]), (void *)self,
            self.base,
            self.stops];
}

- (BOOL)isEqual:(MGLStyleFunction *)other {
    return ([other isKindOfClass:[self class]]
            && (other.base == self.base || other.base == self.base)
            && [other.stops isEqualToDictionary:self.stops]);
}

- (NSUInteger)hash {
    return self.base + self.stops.hash;
}

@end
