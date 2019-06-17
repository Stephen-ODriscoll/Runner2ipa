# Runner2ipa
Convert the Runner.app artifact produced by CodeMagic.io into a Payload.ipa file that can be put on ios devices with Cydia Impactor.

.

.

What is CodeMagic.io?

CodeMagic.io is a Continuous Integration and Development tool exclusively for Flutter. It can build a Flutter project, test and deploy directly to both Apple App Store and Google Play Store. I use it to get Flutter apps on my iPhone without needing a Mac. Codemagic produces an artifact called Runner.app which can be converted into a .ipa file and then sent to IOS devices with Cydia Impactor. For simplicity, I’ve created an executable called Runner2ipa to do the conversion but the steps are here if you want them: https://medium.com/flutter-community/developing-and-debugging-flutter-apps-for-ios-without-a-mac-8d362a8ec667

.

.

How do I get apps on my IOS device?

Requirements:

•	iTunes (https://www.apple.com/ie/itunes/download)

•	Cydia Impactor (http://www.cydiaimpactor.com)

•	Runner.app (Produced by CodeMagic.io)

•	runner2ipa.exe (Available here)

Steps:

1.)	Open iTunes and make sure your device is connected

2.)	Run Runner2ipa.exe and select Runner.app, this will build Payload.ipa

3.)	Run Cydia Impactor (not as admin), your device should be recognised

4.)	Drag Payload.ipa into Impactor and sign it with any apple id

5.)	On your device: Tap Settings > General > Profiles or Profiles & Device Management > your apple id > Trust