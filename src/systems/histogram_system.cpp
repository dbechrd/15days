#include "histogram_system.h"
#include "../facets/depot.h"

void HistogramSystem::Update(double now, Depot &depot)
{
    // TODO: Animate it updating to make it look nice, whatever that means
    for (Histogram &histo : depot.histogram) {
        histo.values.pop_front();
        histo.values.push_back((float)depot.FpsSmooth());
    }
}

void HistogramSystem::Display(double now, Depot &depot, DrawQueue &drawQueue)
{
    for (Histogram &histo : depot.histogram) {
        Position *position = (Position *)depot.GetFacet(histo.uid, Facet_Position);
        DLB_ASSERT(position);
        if (!position) {
            printf("WARN: Can't draw a histogram with no position");
            continue;
        }

        float histoWidth = 200.0f;
        float barPadding = 1.0f;
        //float bars = (float)histo.values.size();
        //float barWidth = (histoWidth - (barPadding * (bars - 1))) / bars;
        float barWidth = 1.0f;
        float histoHeight = 20.0f;

        float maxValue = 0.0f;
        for (float value : histo.values) {
            maxValue = MAX(maxValue, value);
        }

        float barScale = histoHeight / maxValue;

        vec2 cursor{};
        cursor.x = position->pos.x;
        cursor.y = position->pos.y;

        for (float value : histo.values) {
            float bottom = cursor.y + histoHeight;
            float height = value * barScale;

            rect rect{};
            rect.x = cursor.x;
            rect.y = bottom - height;
            rect.w = barWidth;
            rect.h = height;

            DrawCommand drawCmd{};
            drawCmd.uid = histo.uid;
            drawCmd.color = C255(COLOR_LIME);
            drawCmd.dstRect = rect;
            drawQueue.push(drawCmd);

            cursor.x += barWidth + barPadding;
        }
    }
}