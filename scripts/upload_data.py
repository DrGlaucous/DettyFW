Import( 'env', 'projenv' )

print("hello, there")

#pio run -t uploadfs
#C:\.platformio\penv\Scripts\platformio.exe run --target uploadfs --environment lolin32_lite

#def before_upload(source, target, env):
#    print("=========================hi, there")

#def after_upload(source, target, env):
#    print("=========================bye, there")



#env.AddPreAction( '$BUILD_DIR/${ESP32_SPIFFS_IMAGE_NAME}.bin', after_upload )