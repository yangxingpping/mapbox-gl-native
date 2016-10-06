#include <benchmark/benchmark.h>

#include <mbgl/benchmark/util.hpp>
#include <mbgl/map/map.hpp>
#include <mbgl/platform/default/headless_backend.hpp>
#include <mbgl/platform/default/headless_view.hpp>
#include <mbgl/sprite/sprite_image.hpp>
#include <mbgl/storage/default_file_source.hpp>
#include <mbgl/storage/network_status.hpp>
#include <mbgl/util/image.hpp>
#include <mbgl/util/io.hpp>
#include <mbgl/util/run_loop.hpp>

using namespace mbgl;

namespace {

class QueryBenchmark {
public:
    QueryBenchmark() {
        NetworkStatus::Set(NetworkStatus::Status::Offline);
        fileSource.setAccessToken("foobar");

        map.setStyleJSON(util::read_file("benchmark/fixtures/api/query_style.json"));
        map.setLatLngZoom({ 40.726989, -73.992857 }, 15); // Manhattan

        auto decoded = decodeImage(util::read_file("benchmark/fixtures/api/default_marker.png"));
        auto image = std::make_unique<SpriteImage>(std::move(decoded), 1.0);
        map.addImage("test-icon", std::move(image));

        view.resize(1000, 1000);

        mbgl::benchmark::render(map);
    }

    util::RunLoop loop;
    HeadlessBackend backend;
    HeadlessView view;
    DefaultFileSource fileSource{ "benchmark/fixtures/api/cache.db", "." };
    Map map{ backend, view, view.getPixelRatio(), fileSource, MapMode::Still };
    ScreenBox box{{ 0, 0 }, { 1000, 1000 }};
};

} // end namespace

static void API_queryRenderedFeaturesAll(::benchmark::State& state) {
    QueryBenchmark bench;

    while (state.KeepRunning()) {
        bench.map.queryRenderedFeatures(bench.box);
    }
}

static void API_queryRenderedFeaturesLayerFromLowDensity(::benchmark::State& state) {
    QueryBenchmark bench;

    while (state.KeepRunning()) {
        bench.map.queryRenderedFeatures(bench.box, {{ "testlayer" }});
    }
}

static void API_queryRenderedFeaturesLayerFromHighDensity(::benchmark::State& state) {
    QueryBenchmark bench;

    while (state.KeepRunning()) {
        bench.map.queryRenderedFeatures(bench.box, {{ "road-street" }});
    }
}

BENCHMARK(API_queryRenderedFeaturesAll);
BENCHMARK(API_queryRenderedFeaturesLayerFromLowDensity);
BENCHMARK(API_queryRenderedFeaturesLayerFromHighDensity);
