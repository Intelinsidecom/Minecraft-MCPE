#include "App.h"
#include "AppPlatform_android.h"
#include "platform/input/Multitouch.h"

// Horrible, I know. / A
#ifndef MAIN_CLASS
#include "main.cpp"
#endif

#include <pthread.h>

// References for JNI
static pthread_mutex_t g_activityMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_renderMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_activityCond = PTHREAD_COND_INITIALIZER;
static jobject g_pActivity  = 0;
static AppPlatform_android appPlatform;
static App* gApp = 0;
static AppContext gContext;

static void setupExternalPath(JNIEnv* env, jobject activity, MAIN_CLASS* app)
{
    LOGI("setupExternalPath");

    jclass versionClass = env->FindClass("android/os/Build$VERSION");
    jfieldID sdkIntField = env->GetStaticFieldID(versionClass, "SDK_INT", "I");
    int sdkInt = env->GetStaticIntField(versionClass, sdkIntField);
    LOGI("Android SDK_INT: %d", sdkInt);

    jclass clazz = env->FindClass("android/os/Environment");
    jmethodID method = env->GetStaticMethodID(clazz, "getExternalStorageDirectory", "()Ljava/io/File;");
    jobject file = env->CallStaticObjectMethod(clazz, method);

    jclass fileClass = env->GetObjectClass(file);
    jmethodID fileMethod = env->GetMethodID(fileClass, "getAbsolutePath", "()Ljava/lang/String;");
    jobject pathString = env->CallObjectMethod(file, fileMethod);

    const char* str = env->GetStringUTFChars((jstring) pathString, NULL);
    std::string path = str;
    
    if (sdkInt >= 29) {
        jclass contextClass = env->FindClass("android/content/Context");
        jmethodID getExternalFilesDir = env->GetMethodID(contextClass, "getExternalFilesDir", "(Ljava/lang/String;)Ljava/io/File;");
        jobject externalFilesDir = env->CallObjectMethod(activity, getExternalFilesDir, NULL);
        
        if (externalFilesDir) {
            jclass efc = env->GetObjectClass(externalFilesDir);
            jmethodID gapm = env->GetMethodID(efc, "getAbsolutePath", "()Ljava/lang/String;");
            jobject ps = env->CallObjectMethod(externalFilesDir, gapm);
            const char* filesPath = env->GetStringUTFChars((jstring) ps, NULL);
            path = filesPath;
            env->ReleaseStringUTFChars((jstring)ps, filesPath);
            LOGI("Using app-specific external files directory: %s", path.c_str());
        } else {
            path = "/data/data/com.mojang.minecraftpe/files";
            LOGI("Fallback to internal storage: %s", path.c_str());
        }
    }

    app->externalStoragePath = path;
    app->externalCacheStoragePath = path;
    LOGI("Final externalStoragePath: %s", path.c_str());

    env->ReleaseStringUTFChars((jstring)pathString, str);
}

extern "C" {
JNIEXPORT jint JNICALL
JNI_OnLoad( JavaVM * vm, void * reserved )
{
    LOGI("Entering OnLoad\n");
    return appPlatform.init(vm);
}

JNIEXPORT void JNICALL
Java_com_mojang_minecraftpe_MainActivity_nativeRegisterThis(JNIEnv* env, jobject activity) {
    LOGI("@RegisterThis\n");
    pthread_mutex_lock(&g_activityMutex);
    g_pActivity = (jobject)env->NewGlobalRef( activity );
    pthread_cond_broadcast(&g_activityCond);
    pthread_mutex_unlock(&g_activityMutex);
}

JNIEXPORT void JNICALL
Java_com_mojang_minecraftpe_MainActivity_nativeUnregisterThis(JNIEnv* env, jobject activity) {
    LOGI("@UnregisterThis\n");
    env->DeleteGlobalRef( g_pActivity );
}

JNIEXPORT void JNICALL
Java_com_mojang_minecraftpe_MainActivity_nativeStopThis(JNIEnv* env, jobject activity) {
    LOGI("@nativeStopThis\n");
}

JNIEXPORT void JNICALL
Java_com_mojang_minecraftpe_MainActivity_nativeOnCreate(JNIEnv* env, jobject activity) {
    LOGI("@nativeOnCreate\n");

    pthread_mutex_lock(&g_activityMutex);
    appPlatform.instance = g_pActivity;
    appPlatform.initConsts();
    gContext.doRender = false;
    gContext.platform = &appPlatform;

    if (!gApp) {
        gApp = new MAIN_CLASS();
        setupExternalPath(env, g_pActivity, (MAIN_CLASS*)gApp);
    }
    pthread_cond_broadcast(&g_activityCond);
    pthread_mutex_unlock(&g_activityMutex);
}

JNIEXPORT void JNICALL
Java_com_mojang_minecraftpe_GLRenderer_nativeOnSurfaceCreated(JNIEnv* env) {
    LOGI("@nativeOnSurfaceCreated\n");

     if (gApp) {
//          gApp->setSize( gContext.platform->getScreenWidth(),
//                         gContext.platform->getScreenHeight(),
//                         gContext.platform->isTouchscreen());

         // Don't call onGraphicsReset the first time
        if (gApp->isInited())
            gApp->onGraphicsReset(gContext);

        if (!gApp->isInited())
            gApp->init(gContext);
     }
}

JNIEXPORT void JNICALL
Java_com_mojang_minecraftpe_GLRenderer_nativeOnSurfaceChanged(JNIEnv* env, jclass cls, jint w, jint h) {
    LOGI("@nativeOnSurfaceChanged: %p\n", pthread_self());

    if (gApp) {
        gApp->setSize(w, h);

        if (!gApp->isInited())
            gApp->init(gContext);

        if (!gApp->isInited())
            LOGI("nativeOnSurfaceChanged: NOT INITED!\n");
    }
}

JNIEXPORT void JNICALL
Java_com_mojang_minecraftpe_MainActivity_nativeOnDestroy(JNIEnv* env, jobject activity) {
    LOGI("@nativeOnDestroy\n");

    delete gApp;
    gApp = 0;
    //gApp->onGraphicsReset(gContext);
}

JNIEXPORT void JNICALL
Java_com_mojang_minecraftpe_GLRenderer_nativeUpdate(JNIEnv* env, jclass cls) {
    //LOGI("@nativeUpdate: %p\n", pthread_self());
    if (gApp) {
        if (!gApp->isInited())
            gApp->init(gContext);

        gApp->update();

        if (gApp->wantToQuit())
            appPlatform.finish();
    }
}

//
// Keyboard events
//
JNIEXPORT void JNICALL
Java_com_mojang_minecraftpe_MainActivity_nativeOnKeyDown(JNIEnv* env, jclass cls, jint keyCode) {
    LOGI("@nativeOnKeyDown: %d\n", keyCode);
    Keyboard::feed(keyCode, true);
}
JNIEXPORT void JNICALL
Java_com_mojang_minecraftpe_MainActivity_nativeOnKeyUp(JNIEnv* env, jclass cls, jint keyCode) {
    LOGI("@nativeOnKeyUp: %d\n", (int)keyCode);
    Keyboard::feed(keyCode, false);
}

JNIEXPORT jboolean JNICALL
Java_com_mojang_minecraftpe_MainActivity_nativeHandleBack(JNIEnv* env, jobject activity, jboolean isDown) {
    LOGI("@nativeHandleBack: %d\n", isDown);
    if (gApp) return (gApp->handleBack(isDown)) ? JNI_TRUE : JNI_FALSE;
    return JNI_FALSE;
}

//
// Mouse events
//
JNIEXPORT void JNICALL
Java_com_mojang_minecraftpe_MainActivity_nativeMouseDown(JNIEnv* env, jclass cls, jint pointerId, jint buttonId, jfloat x, jfloat y) {
    //LOGI("@nativeMouseDown: %f %f\n", x, y);
    mouseDown(1, x, y);
    pointerDown(pointerId, x, y);
}
JNIEXPORT void JNICALL
Java_com_mojang_minecraftpe_MainActivity_nativeMouseUp(JNIEnv* env, jclass cls, jint pointerId, jint buttonId, jfloat x, jfloat y) {
    //LOGI("@nativeMouseUp: %f %f\n", x, y);
    mouseUp(1, x, y);
    pointerUp(pointerId, x, y);
}
JNIEXPORT void JNICALL
Java_com_mojang_minecraftpe_MainActivity_nativeMouseMove(JNIEnv* env, jclass cls, jint pointerId, jfloat x, jfloat y) {
    //LOGI("@nativeMouseMove: %f %f\n", x, y);
    mouseMove(x, y);
    pointerMove(pointerId, x, y);
}
}

static void internal_process_input(struct android_app* app, struct android_poll_source* source) {
	AInputEvent* event = NULL;
	if (AInputQueue_getEvent(app->inputQueue, &event) >= 0) {
		LOGV("New input event: type=%d\n", AInputEvent_getType(event));
		bool isBackButtonDown = AKeyEvent_getKeyCode(event) == AKEYCODE_BACK && AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN;
		if(!(appPlatform.isKeyboardVisible() && isBackButtonDown)) {
			if (AInputQueue_preDispatchEvent(app->inputQueue, event)) {
				return;
			}
		}
		int32_t handled = 0;
		if (app->onInputEvent != NULL) handled = app->onInputEvent(app, event);
		AInputQueue_finishEvent(app->inputQueue, event, handled);
	} else {
		LOGE("Failure reading next input event: %s\n", strerror(errno));
	}
}

void
android_main( struct android_app* state )
{
    struct ENGINE engine;

    // Make sure glue isn't stripped.
    app_dummy();

    memset( (void*)&engine, 0, sizeof(engine) );
    state->userData     = (void*)&engine;
    state->onAppCmd     = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    state->destroyRequested = 0;

    pthread_mutex_lock(&g_activityMutex);
    while (g_pActivity == NULL || gApp == NULL) {
        LOGI("Waiting for Activity registration and gApp initialization...");
        pthread_cond_wait(&g_activityCond, &g_activityMutex);
    }
    appPlatform.instance = g_pActivity;
    App* app = gApp;
    pthread_mutex_unlock(&g_activityMutex);

    appPlatform.initConsts();

    //LOGI("socket-stuff\n");
    //socketDesc = initSocket(1999);

    engine.userApp      = app;
    engine.app          = state;
    engine.is_inited    = false;
    engine.appContext.doRender = true;
    engine.appContext.platform = &appPlatform;

    if( state->savedState != NULL )
    {
        // We are starting with a previous saved state; restore from it.
       app->loadState(state->savedState, state->savedStateSize);
    }

    bool inited = false;
    bool teardownPhase = false;
	appPlatform._nativeActivity = state->activity;
    // our 'main loop'
    while( 1 )
    {
        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source* source;
		
        while( (ident = ALooper_pollAll( 0, NULL, &events, (void**)&source) ) >= 0 )
        {
            // Process this event.
            // This will call the function pointer android_app::onInputEvent() which in our case is
            // engine_handle_input()
            if( source != NULL ) {
                if(source->id == 2) {
					// Back button is intercepted by the ime on android 4.1/4.2 resulting in the application stopping to respond.
					internal_process_input( state, source );
				} else {
					source->process( state, source );
				}
            }

        }
         // Check if we are exiting.
         if( state->destroyRequested )
         {
             //engine_term_display( &engine );
             delete app;
             return;
         }

		 if (!inited && engine.is_inited) {
			 app->init(engine.appContext);
			 app->setSize(engine.width, engine.height);
			 inited = true;
		 }

        if (inited && engine.is_inited && engine.has_focus) {
            // app->update();
        } else {
            sleepMs(50);
        }

        if (!teardownPhase && app->wantToQuit()) {
            teardownPhase = true;
            LOGI("tearing down!");
            ANativeActivity_finish(state->activity);
        }
    }
}
