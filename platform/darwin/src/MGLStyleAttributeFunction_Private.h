#import "MGLStyleAttributeFunction.h"

#include <mbgl/util/color.hpp>
#include <mbgl/util/variant.hpp>
#include <mbgl/style/function.hpp>

#include <mbgl/style/property_value.hpp>

#define MGLSetEnumProperty(name, Name, MBGLType, ObjCType) \
    if (name.isFunction) { \
        NSAssert([name isKindOfClass:[MGLStyleAttributeFunction class]], @"" #name @" should be a function"); \
        \
        __block std::vector<std::pair<float, mbgl::style::MBGLType>> stops; \
        [[(MGLStyleAttributeFunction *)name stops] enumerateKeysAndObjectsUsingBlock:^(NSNumber * _Nonnull key, NSValue * _Nonnull obj, BOOL * _Nonnull stop) { \
            NSAssert([obj isKindOfClass:[NSValue class]], @"Stops in " #name @" should be NSValues"); \
            ObjCType value; \
            [obj getValue:&value]; \
            stops.emplace_back(key.floatValue, static_cast<mbgl::style::MBGLType>(value)); \
        }]; \
        auto function = mbgl::style::Function<mbgl::style::MBGLType> { \
            stops, \
            [(MGLStyleAttributeFunction *)name base].floatValue, \
        }; \
        self.layer->set##Name(function); \
    } else if (name) { \
        NSAssert([name isKindOfClass:[NSValue class]], @"" #name @"should be an NSValue"); \
        ObjCType value; \
        [(NSValue *)name getValue:&value]; \
        self.layer->set##Name({ static_cast<mbgl::style::MBGLType>(value) }); \
    } else { \
        self.layer->set##Name({}); \
    }

#define MGLGetEnumProperty(Name, MBGLType, ObjCType) \
    const char *type = @encode(ObjCType); \
    mbgl::style::PropertyValue<mbgl::style::MBGLType> property = self.layer->get##Name() ?: self.layer->getDefault##Name(); \
    if (property.isConstant()) { \
        ObjCType value = static_cast<ObjCType>(property.asConstant()); \
        return [NSValue value:&value withObjCType:type]; \
    } else if (property.isFunction()) { \
        MGLStyleAttributeFunction *function = [[MGLStyleAttributeFunction alloc] init]; \
        auto stops = property.asFunction().getStops(); \
        NSMutableDictionary *convertedStops = [NSMutableDictionary dictionaryWithCapacity:stops.size()]; \
        for (auto stop : stops) { \
            ObjCType value = static_cast<ObjCType>(stop.second); \
            convertedStops[@(stop.first)] = [NSValue value:&value withObjCType:type]; \
        } \
        function.base = @(property.asFunction().getBase()); \
        function.stops = convertedStops; \
        return function; \
    } else { \
        return nil; \
    }

@interface MGLStyleAttributeFunction(Private)

+ (instancetype)functionWithColorPropertyValue:(mbgl::style::Function<mbgl::Color>)property;

+ (instancetype)functionWithNumberPropertyValue:(mbgl::style::Function<float>)property;

+ (instancetype)functionWithBoolPropertyValue:(mbgl::style::Function<bool>)property;

+ (instancetype)functionWithStringPropertyValue:(mbgl::style::Function<std::string>)property;

+ (instancetype)functionWithOffsetPropertyValue:(mbgl::style::Function<std::array<float, 2>>)property;

+ (instancetype)functionWithPaddingPropertyValue:(mbgl::style::Function<std::array<float, 4>>)property;

+ (instancetype)functionWithStringArrayPropertyValue:(mbgl::style::Function<std::vector<std::string>>)property;

+ (instancetype)functionWithNumberArrayPropertyValue:(mbgl::style::Function<std::vector<float>>)property;

+ (instancetype)functionWithEnumProperyValue:(mbgl::style::Function<bool>)property type:(const char *)type;

@end

// Enumerations
template <typename T, typename U, class = typename std::enable_if<std::is_enum<T>::value>::type>
U MGLRawStyleValueFromMBGLValue(const T &mbglStopValue) {
    auto rawValue = static_cast<T>(mbglStopValue);
    return [NSValue value:&rawValue withObjCType:@encode(U)];
}

//MGLColor *MGLRawStyleValueFromMBGLValue(const mbgl::Color mbglStopValue) {
//    return [MGLColor mbgl_colorWithColor:mbglStopValue];
//}

template <typename T, typename U>
U MGLRawStyleValueFromMBGLValue(const std::vector<T> &mbglStopValue) {
    NSMutableArray *array = [NSMutableArray arrayWithCapacity:mbglStopValue.size()];
    for (const auto &mbglElement: mbglStopValue) {
        [array addObject:MGLRawStyleValueFromMBGLValue(mbglElement)];
    }
    return array;
}

//NSNumber *MGLRawStyleValueFromMBGLValue(const float mbglStopValue) {
//    return @(mbglStopValue);
//}
//
//NSNumber *MGLRawStyleValueFromMBGLValue(const bool mbglStopValue) {
//    return @(mbglStopValue);
//}
//
//NSString *MGLRawStyleValueFromMBGLValue(const std::string &mbglStopValue) {
//    return @(mbglStopValue.c_str());
//}

// Enumerations
template <typename T, typename U, class = typename std::enable_if<std::is_enum<U>::value>::type>
void MGLGetMBGLValueFromMGLRawStyleValue(T rawValue, U &mbglValue) {
    [rawValue getValue:&mbglValue];
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
//        [function.stops enumerateKeysAndObjectsUsingBlock:^(NSNumber * _Nonnull zoomKey, NSNumber * _Nonnull stops, BOOL * _Nonnull stop) {
//            NSCAssert([stops isKindOfClass:[NSNumber class]], @"Stops should be NSNumbers");
//            mbglStops.emplace_back(zoomKey.floatValue, stops.floatValue);
//        }];
//        return mbgl::style::Function<U>({{mbglStops}}, function.base.floatValue);
        return mbgl::style::Function<U>({{mbglStops}}, 0.5f);
    } else {
        return {};
    }
}