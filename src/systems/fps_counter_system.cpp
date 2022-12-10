#include "fps_counter_system.h"
#include "../facets/depot.h"

void FpsCounterSystem::React(double now, Depot &depot)
{
    size_t size = depot.msgQueue.size();
    for (int i = 0; i < size; i++) {
        Message msg = depot.msgQueue[i];

        switch (msg.type) {
            case MsgType_Render_FrameBegin: {
                // Update FPS counter text
                for (FpsCounter &fpsCounter : depot.fpsCounter) {
                    float dt = msg.data.render_framebegin.realDtSmooth;
                    size_t fpsCounterMaxLen = 32;
                    char *fpsCounterBuf = (char *)depot.frameArena.Alloc(fpsCounterMaxLen);
                    if (fpsCounterBuf) {
                        snprintf(fpsCounterBuf, fpsCounterMaxLen, "%.2f fps (%.2f ms)", 1.0f / dt, dt * 1000.0f);

                        Message updateText{};
                        updateText.uid = fpsCounter.uid;
                        updateText.type = MsgType_Text_UpdateText;
                        updateText.data.text_updatetext.str = fpsCounterBuf;
                        updateText.data.text_updatetext.color = C255(COLOR_WHITE);
                        depot.msgQueue.push_back(updateText);
                    } else {
                        printf("WARN: Failed to allocate enough frame arena space for fps counter string\n");
                    }
                }
                break;
            }
            default: break;
        }
    }
}