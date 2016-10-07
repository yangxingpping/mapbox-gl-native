#import "MGLStyleAttributeFunction.h"

#import "MGLStyleLayer_Private.h"
#import "MGLStyleAttributeValue_Private.h"
#import "MGLStyleAttributeFunction_Private.h"

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
#warning Hack: referencing this function in this file forces the template specialization to be linked in.
    MBGLValueFromMGLStyleValue<NSNumber *, float>([MGLStyleConstantValue<NSNumber *> valueWithRawValue:@(3)]);
    return nil;
}

@end

NSNumber *MGLRawStyleValueFromMBGLValue(const bool mbglStopValue) {
    return @(mbglStopValue);
}

NSNumber *MGLRawStyleValueFromMBGLValue(const float mbglStopValue) {
    return @(mbglStopValue);
}

NSString *MGLRawStyleValueFromMBGLValue(const std::string &mbglStopValue) {
    return @(mbglStopValue.c_str());
}

// Offsets
NSValue *MGLRawStyleValueFromMBGLValue(const std::array<float, 2> &mbglStopValue) {
    return [NSValue mgl_valueWithOffsetArray:mbglStopValue];
}

// Padding
NSValue *MGLRawStyleValueFromMBGLValue(const std::array<float, 4> &mbglStopValue) {
    return [NSValue mgl_valueWithPaddingArray:mbglStopValue];
}

// Enumerations
template <typename T, typename U, class = typename std::enable_if<std::is_enum<T>::value>::type>
U MGLRawStyleValueFromMBGLValue(const T &mbglStopValue) {
    auto rawValue = static_cast<T>(mbglStopValue);
    return [NSValue value:&rawValue withObjCType:@encode(U)];
}

MGLColor *MGLRawStyleValueFromMBGLValue(const mbgl::Color mbglStopValue) {
    return [MGLColor mbgl_colorWithColor:mbglStopValue];
}

template <typename T, typename U>
U MGLRawStyleValueFromMBGLValue(const std::vector<T> &mbglStopValue) {
    NSMutableArray *array = [NSMutableArray arrayWithCapacity:mbglStopValue.size()];
    for (const auto &mbglElement: mbglStopValue) {
        [array addObject:MGLRawStyleValueFromMBGLValue(mbglElement)];
    }
    return array;
}

template <typename T, typename U>
MGLStyleConstantValue<U> *MGLStyleConstantValueFromMBGLValue(const T mbglValue) {
    auto rawValue = MGLRawStyleValueFromMBGLValue(mbglValue);
    return [MGLStyleConstantValue<U> valueWithRawValue:rawValue];
}

template <typename T, typename U>
MGLStyleFunction<U> *MGLStyleFunctionFromMBGLFunction(const mbgl::style::Function<T> &mbglFunction) {
    const auto &mbglStops = mbglFunction.getStops();
    NSMutableDictionary *stops = [NSMutableDictionary dictionaryWithCapacity:mbglStops.size()];
    for (const auto &mbglStop : mbglStops) {
        auto rawValue = MGLRawStyleValueFromMBGLValue(mbglStop.second);
        stops[@(mbglStop.first)] = [MGLStyleValue valueWithRawValue:rawValue];
    }
    return [MGLStyleFunction<U> functionWithBase:mbglFunction.getBase() stops:stops];
}

template <typename T, typename U>
MGLStyleValue<U> *MGLStyleValueFromMBGLValue(const mbgl::style::PropertyValue<T> &mbglValue) {
    if (mbglValue.isConstant()) {
        return MGLStyleConstantValueFromMBGLValue<T, U>(mbglValue.asConstant());
    } else if (mbglValue.isFunction()) {
        return MGLStyleFunctionFromMBGLFunction<T, U>(mbglValue.asFunction());
    } else {
        return nil;
    }
}

void MGLGetMBGLValueFromMGLRawStyleValue(NSNumber *rawValue, bool &mbglValue) {
    mbglValue = !!rawValue.boolValue;
}

void MGLGetMBGLValueFromMGLRawStyleValue(NSNumber *rawValue, float &mbglValue) {
    mbglValue = !!rawValue.boolValue;
}

void MGLGetMBGLValueFromMGLRawStyleValue(NSString *rawValue, std::string &mbglValue) {
    mbglValue = rawValue.UTF8String;
}

// Offsets
void MGLGetMBGLValueFromMGLRawStyleValue(NSValue *rawValue, std::array<float, 2> &mbglValue) {
    mbglValue = rawValue.mgl_offsetArrayValue;
}

// Padding
void MGLGetMBGLValueFromMGLRawStyleValue(NSValue *rawValue, std::array<float, 4> &mbglValue) {
    mbglValue = rawValue.mgl_paddingArrayValue;
}

// Enumerations
template <typename T, typename U, class = typename std::enable_if<std::is_enum<U>::value>::type>
void MGLGetMBGLValueFromMGLRawStyleValue(T rawValue, U &mbglValue) {
    [rawValue getValue:&mbglValue];
}

void MGLGetMBGLValueFromMGLRawStyleValue(MGLColor *rawValue, mbgl::Color &mbglValue) {
    mbglValue = rawValue.mbgl_color;
}

template <typename T, typename U>
void MGLGetMBGLValueFromMGLRawStyleValue(T rawValue, std::vector<U> &mbglValue) {
    mbglValue.reserve(rawValue.count);
    for (id obj in rawValue) {
        U mbglElement;
        mbglValue.push_back(MGLGetMBGLValueFromMGLRawStyleValue(obj, mbglElement));
    }
}

template <typename T, typename U>
mbgl::style::PropertyValue<U> MBGLValueFromMGLStyleValue(MGLStyleValue<T> *value) {
    if ([value isKindOfClass:[MGLStyleConstantValue class]]) {
        U mbglValue;
        MGLGetMBGLValueFromMGLRawStyleValue([(MGLStyleConstantValue<T> *)value rawValue], mbglValue);
        return mbglValue;
    } else if ([value isKindOfClass:[MGLStyleFunction class]]) {
        MGLStyleFunction<T> *function = (MGLStyleFunction<T> *)value;
        __block std::vector<std::pair<float, U>> mbglStops;
        [function.stops enumerateKeysAndObjectsUsingBlock:^(NSNumber * _Nonnull zoomKey, MGLStyleValue<NSNumber *> * _Nonnull stopValue, BOOL * _Nonnull stop) {
            NSCAssert([stopValue isKindOfClass:[MGLStyleValue class]], @"Stops should be MGLStyleValues");
            auto mbglStopValue = MBGLValueFromMGLStyleValue<T, U>(stopValue);
            NSCAssert(mbglStopValue.isConstant(), @"Stops must be constant");
            mbglStops.emplace_back(zoomKey.floatValue, mbglStopValue.asConstant());
        }];
        return mbgl::style::Function<U>({{mbglStops}}, function.base);
    } else {
        return {};
    }
}





@interface MGLStyleAttributeFunction() <MGLStyleAttributeValue_Private>
@end

@implementation MGLStyleAttributeFunction

- (instancetype)init
{
    if (self = [super init]) {
        _base = @1;
    }
    return self;
}

- (BOOL)isFunction
{
    return YES;
}

- (mbgl::style::PropertyValue<mbgl::Color>)mbgl_colorPropertyValue
{
    __block std::vector<std::pair<float, mbgl::Color>> stops;
    [self.stops enumerateKeysAndObjectsUsingBlock:^(NSNumber * _Nonnull zoomKey, MGLColor * _Nonnull color, BOOL * _Nonnull stop) {
        NSAssert([color isKindOfClass:[MGLColor class]], @"Stops should be colors");
        stops.emplace_back(zoomKey.floatValue, color.mbgl_color);
    }];
    return mbgl::style::Function<mbgl::Color>({{stops}}, _base.floatValue);
}

- (mbgl::style::PropertyValue<float>)mbgl_floatPropertyValue
{
    __block std::vector<std::pair<float, float>> stops;
    [self.stops enumerateKeysAndObjectsUsingBlock:^(NSNumber * _Nonnull zoomKey, NSNumber * _Nonnull number, BOOL * _Nonnull stop) {
        NSAssert([number isKindOfClass:[NSNumber class]], @"Stops should be NSNumbers");
        stops.emplace_back(zoomKey.floatValue, number.floatValue);
    }];
    return mbgl::style::Function<float>({{stops}}, _base.floatValue);
}

- (mbgl::style::PropertyValue<bool>)mbgl_boolPropertyValue
{
    __block std::vector<std::pair<float, bool>> stops;
    [self.stops enumerateKeysAndObjectsUsingBlock:^(NSNumber * _Nonnull zoomKey, NSNumber * _Nonnull number, BOOL * _Nonnull stop) {
        NSAssert([number isKindOfClass:[NSNumber class]], @"Stops should be NSNumbers");
        stops.emplace_back(zoomKey.floatValue, number.boolValue);
    }];
    return mbgl::style::Function<bool>({{stops}}, _base.floatValue);
}

- (mbgl::style::PropertyValue<std::string>)mbgl_stringPropertyValue
{
    __block std::vector<std::pair<float, std::string>> stops;
    [self.stops enumerateKeysAndObjectsUsingBlock:^(NSNumber * _Nonnull zoomKey, NSString * _Nonnull string, BOOL * _Nonnull stop) {
        NSAssert([string isKindOfClass:[NSString class]], @"Stops should be strings");
        stops.emplace_back(zoomKey.floatValue, string.UTF8String);
    }];
    return mbgl::style::Function<std::string>({{stops}}, _base.floatValue);
}

- (mbgl::style::PropertyValue<std::vector<std::string> >)mbgl_stringArrayPropertyValue
{
    __block std::vector<std::pair<float, std::vector<std::string>>> stops;
    [self.stops enumerateKeysAndObjectsUsingBlock:^(NSNumber * _Nonnull zoomKey, NSArray *  _Nonnull strings, BOOL * _Nonnull stop) {
        NSAssert([strings isKindOfClass:[NSArray class]], @"Stops should be NSArray");
        std::vector<std::string>convertedStrings;
        for (NSString *string in strings) {
            convertedStrings.emplace_back(string.UTF8String);
        }
        stops.emplace_back(zoomKey.floatValue, convertedStrings);
    }];
    return mbgl::style::Function<std::vector<std::string>>({{stops}}, _base.floatValue);
}

- (mbgl::style::PropertyValue<std::vector<float> >)mbgl_numberArrayPropertyValue
{
    __block std::vector<std::pair<float, std::vector<float>>> stops;
    [self.stops enumerateKeysAndObjectsUsingBlock:^(NSNumber * _Nonnull zoomKey, NSArray *  _Nonnull numbers, BOOL * _Nonnull stop) {
        NSAssert([numbers isKindOfClass:[NSArray class]], @"Stops should be NSArray");
        std::vector<float>convertedNumbers;
        for (NSNumber *number in numbers) {
            convertedNumbers.emplace_back(number.floatValue);
        }
        stops.emplace_back(zoomKey.floatValue, convertedNumbers);
    }];
    return mbgl::style::Function<std::vector<float>>({{stops}}, _base.floatValue);
}

- (mbgl::style::PropertyValue<std::array<float, 4> >)mbgl_paddingPropertyValue
{
    __block std::vector<std::pair<float, std::array<float, 4>>> stops;
    [self.stops enumerateKeysAndObjectsUsingBlock:^(NSNumber * _Nonnull zoomKey, NSValue * _Nonnull padding, BOOL * _Nonnull stop) {
        NSAssert([padding isKindOfClass:[NSValue class]], @"Stops should be NSValue");
        stops.emplace_back(zoomKey.floatValue, padding.mgl_paddingArrayValue);
    }];
    return mbgl::style::Function<std::array<float, 4>>({{stops}}, _base.floatValue);
}

- (mbgl::style::PropertyValue<std::array<float, 2> >)mbgl_offsetPropertyValue
{
    __block std::vector<std::pair<float, std::array<float, 2>>> stops;
    [self.stops enumerateKeysAndObjectsUsingBlock:^(NSNumber * _Nonnull zoomKey, NSValue * _Nonnull offset, BOOL * _Nonnull stop) {
        NSAssert([offset isKindOfClass:[NSValue class]], @"Stops should be NSValue");
        stops.emplace_back(zoomKey.floatValue, offset.mgl_offsetArrayValue);
    }];
    return mbgl::style::Function<std::array<float, 2>>({{stops}}, _base.floatValue);
}

+ (instancetype)functionWithColorPropertyValue:(mbgl::style::Function<mbgl::Color>)property
{
    MGLStyleAttributeFunction *function = [[MGLStyleAttributeFunction alloc] init];
    auto stops = property.getStops();
    NSMutableDictionary *convertedStops = [NSMutableDictionary dictionaryWithCapacity:stops.size()];
    for (auto stop : stops) {
        convertedStops[@(stop.first)] = [MGLColor mbgl_colorWithColor:stop.second];
    }
    function.base = @(property.getBase());
    function.stops = convertedStops;
    return function;
}

+ (instancetype)functionWithNumberPropertyValue:(mbgl::style::Function<float>)property
{
    MGLStyleAttributeFunction *function = [[MGLStyleAttributeFunction alloc] init];
    auto stops = property.getStops();
    NSMutableDictionary *convertedStops = [NSMutableDictionary dictionaryWithCapacity:stops.size()];
    for (auto stop : stops) {
        convertedStops[@(stop.first)] = @(stop.second);
    }
    function.base = @(property.getBase());
    function.stops = convertedStops;
    return function;
}

+ (instancetype)functionWithBoolPropertyValue:(mbgl::style::Function<bool>)property
{
    MGLStyleAttributeFunction *function = [[MGLStyleAttributeFunction alloc] init];
    auto stops = property.getStops();
    NSMutableDictionary *convertedStops = [NSMutableDictionary dictionaryWithCapacity:stops.size()];
    for (auto stop : stops) {
        convertedStops[@(stop.first)] = @(stop.second);
    }
    function.base = @(property.getBase());
    function.stops = convertedStops;
    return function;
}

+ (instancetype)functionWithStringPropertyValue:(mbgl::style::Function<std::string>)property
{
    MGLStyleAttributeFunction *function = [[MGLStyleAttributeFunction alloc] init];
    auto stops = property.getStops();
    NSMutableDictionary *convertedStops = [NSMutableDictionary dictionaryWithCapacity:stops.size()];
    for (auto stop : stops) {
        convertedStops[@(stop.first)] = @(stop.second.c_str());
    }
    function.base = @(property.getBase());
    function.stops = convertedStops;
    return function;
}

+ (instancetype)functionWithOffsetPropertyValue:(mbgl::style::Function<std::array<float, 2> >)property
{
    MGLStyleAttributeFunction *function = [[MGLStyleAttributeFunction alloc] init];
    auto stops = property.getStops();
    NSMutableDictionary *convertedStops = [NSMutableDictionary dictionaryWithCapacity:stops.size()];
    for (auto stop : stops) {
        convertedStops[@(stop.first)] = [NSValue mgl_valueWithOffsetArray:stop.second];
    }
    function.base = @(property.getBase());
    function.stops = convertedStops;
    return function;
}

+ (instancetype)functionWithPaddingPropertyValue:(mbgl::style::Function<std::array<float, 4> >)property
{
    MGLStyleAttributeFunction *function = [[MGLStyleAttributeFunction alloc] init];
    auto stops = property.getStops();
    NSMutableDictionary *convertedStops = [NSMutableDictionary dictionaryWithCapacity:stops.size()];
    for (auto stop : stops) {
        convertedStops[@(stop.first)] = [NSValue mgl_valueWithPaddingArray:stop.second];
    }
    function.base = @(property.getBase());
    function.stops = convertedStops;
    return function;
}

+ (instancetype)functionWithStringArrayPropertyValue:(mbgl::style::Function<std::vector<std::string> >)property
{
    MGLStyleAttributeFunction *function = [[MGLStyleAttributeFunction alloc] init];
    auto stops = property.getStops();
    NSMutableDictionary *convertedStops = [NSMutableDictionary dictionaryWithCapacity:stops.size()];
    for (auto stop : stops) {
        auto strings = stop.second;
        NSMutableArray *convertedStrings = [NSMutableArray arrayWithCapacity:strings.size()];
        for (auto const& string: strings) {
            [convertedStrings addObject:@(string.c_str())];
        }
        convertedStops[@(stop.first)] = convertedStrings;
    }
    function.base = @(property.getBase());
    function.stops = convertedStops;
    return function;
}

+ (instancetype)functionWithNumberArrayPropertyValue:(mbgl::style::Function<std::vector<float> >)property
{
    MGLStyleAttributeFunction *function = [[MGLStyleAttributeFunction alloc] init];
    auto stops = property.getStops();
    NSMutableDictionary *convertedStops = [NSMutableDictionary dictionaryWithCapacity:stops.size()];
    for (auto stop : stops) {
        auto numbers = stop.second;
        NSMutableArray *convertedNumbers = [NSMutableArray arrayWithCapacity:numbers.size()];
        for (auto const& number: numbers) {
            [convertedNumbers addObject:@(number)];
        }
        convertedStops[@(stop.first)] = convertedNumbers;
    }
    function.base = @(property.getBase());
    function.stops = convertedStops;
    return function;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: %p, base = %@; stops = %@>",
            NSStringFromClass([self class]), (void *)self,
            self.base,
            self.stops];
}

- (BOOL)isEqual:(MGLStyleAttributeFunction *)other
{
    return ([other isKindOfClass:[self class]]
            && (other.base == self.base || [other.base isEqualToNumber:self.base])
            && [other.stops isEqualToDictionary:self.stops]);
}

- (NSUInteger)hash
{
    return self.base.hash + self.stops.hash;
}

@end
