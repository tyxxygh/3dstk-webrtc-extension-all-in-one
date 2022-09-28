./bin/prepare.sh -platform linux -target webrtc
./bin/buildTarget.sh -target webrtc -platform linux -architecture x64 -configuration all 
./bin/collectLibs.sh
