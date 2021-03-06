#include "app.hpp"
#include "window.hpp"
#include "engine.hpp"
#include "settings.hpp"
#include "asset.hpp"

namespace rack {


struct NewItem : MenuItem {
	void onAction(EventAction &e) override {
		gRackWidget->reset();
	}
};

struct DisconnectItem : MenuItem {
	void onAction(EventAction &e) override {
		gRackWidget->disconnect();
	}
};

struct OpenItem : MenuItem {
	void onAction(EventAction &e) override {
#ifndef ARCH_WEB		
		gRackWidget->openDialog();
#else
		EM_ASM(
		    alert('Drop a .vcv file on the window to open.');
		);
#endif
	}
};

struct SaveItem : MenuItem {
	void onAction(EventAction &e) override {
#ifndef ARCH_WEB		
		gRackWidget->saveDialog();
#else
		gRackWidget->savePatch(assetHidden("autosave.vcv"));
		EM_ASM(
		    FS.syncfs(false, function() {});
		);
#endif
	}
};

#ifdef ARCH_WEB
struct DownloadItem : MenuItem {
	void onAction(EventAction &e) override {
		gRackWidget->savePatch(assetHidden("autosave.vcv"));
		EM_ASM(
		    FS.syncfs(false, function() {
		    	//TODO: use binary
		    	var text = FS.readFile('/work/autosave.vcv', { encoding: 'utf8' });
		    	var blob = new Blob([text], { type:"text/plain;charset=utf-8" });
				saveAs(blob, 'download.vcv', true);
		    });
		);
	}
};
#endif

struct SaveAsItem : MenuItem {
	void onAction(EventAction &e) override {
		gRackWidget->saveAsDialog();
	}
};

struct RevertItem : MenuItem {
	void onAction(EventAction &e) override {
		gRackWidget->revert();
	}
};

struct QuitItem : MenuItem {
	void onAction(EventAction &e) override {
		windowClose();
	}
};

struct FileChoice : ChoiceButton {
	void onAction(EventAction &e) override {
		Menu *menu = gScene->createMenu();
		menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		menu->addChild(MenuItem::create<NewItem>("New", WINDOW_MOD_KEY_NAME "+N"));
		menu->addChild(MenuItem::create<DisconnectItem>("Disconnect Cables"));
		menu->addChild(MenuItem::create<OpenItem>("Open", WINDOW_MOD_KEY_NAME "+O"));
		menu->addChild(MenuItem::create<SaveItem>("Save", WINDOW_MOD_KEY_NAME "+S"));
#ifndef ARCH_WEB
		menu->addChild(MenuItem::create<SaveAsItem>("Save As", WINDOW_MOD_KEY_NAME "+Shift+S"));
		menu->addChild(MenuItem::create<RevertItem>("Revert"));
		menu->addChild(MenuItem::create<QuitItem>("Quit", WINDOW_MOD_KEY_NAME "+Q"));
#else
		menu->addChild(MenuItem::create<DownloadItem>("Save & Download", WINDOW_MOD_KEY_NAME "+Shift+S"));
#endif
	}
};

struct LargerHitBoxesItem : MenuItem {
	void onAction(EventAction &e) override {
		largerHitBoxes = !largerHitBoxes;
		//TODO: save settings
	}
};

struct LockModulesItem : MenuItem {
	void onAction(EventAction &e) override {
		lockModules = !lockModules;
		//TODO: save settings		
	}
};

struct SensitiveKnobsItem : MenuItem {
	void onAction(EventAction &e) override {
		knobSensitivity = (isNear(knobSensitivity, KNOB_SENSITIVITY) ? KNOB_SENSITIVITY * 2 : KNOB_SENSITIVITY);
		//TODO: save settings		
	}
};

struct OptionsChoice : ChoiceButton {
	void onAction(EventAction &e) override {
		Menu *menu = gScene->createMenu();
		menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		menu->addChild(MenuItem::create<LargerHitBoxesItem>("Larger Hit Boxes", CHECKMARK(largerHitBoxes)));
		menu->addChild(MenuItem::create<LockModulesItem>("Lock Modules", CHECKMARK(lockModules)));
		menu->addChild(MenuItem::create<SensitiveKnobsItem>("Sensitive Knobs", CHECKMARK(!isNear(knobSensitivity, KNOB_SENSITIVITY))));
	}
};

struct EnginePauseItem : MenuItem {
	void onAction(EventAction &e) override {
		gPaused = !gPaused;
	}
};

struct EngineSampleRateItem : MenuItem {
	float sampleRate;
	void onAction(EventAction &e) override {
		engineSetSampleRate(sampleRate);
		gPaused = false;
	}
};

struct EngineSampleRateChoice : ChoiceButton {
	void onAction(EventAction &e) override {
		Menu *menu = gScene->createMenu();
		menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		// EnginePauseItem *pauseItem = new EnginePauseItem();
		// pauseItem->text = gPaused ? "Resume engine" : "Pause engine";
		// menu->addChild(pauseItem);

		std::vector<float> sampleRates = {44100, 48000, 88200, 96000, 176400, 192000};
		for (float sampleRate : sampleRates) {
			EngineSampleRateItem *item = new EngineSampleRateItem();
			item->text = stringf("%.0f Hz", sampleRate);
			item->sampleRate = sampleRate;
			menu->addChild(item);
		}
	}
	void step() override {
		if (gPaused)
			text = "Paused";
		else
			text = stringf("%.0f Hz", engineGetSampleRate());
	}
};


Toolbar::Toolbar() {
	box.size.y = BND_WIDGET_HEIGHT + 2*5;

	layoutLeft = new SequentialLayout();
	layoutLeft->padding = Vec(5, 5);
	layoutLeft->spacing = 5;
	addChild(layoutLeft);

	ChoiceButton *fileChoice = new FileChoice();
	fileChoice->box.size.x = 100;
	fileChoice->text = "File";
	layoutLeft->addChild(fileChoice);

	ChoiceButton *optionsChoice = new OptionsChoice();
	optionsChoice->box.size.x = 100;
	optionsChoice->text = "Options";
	layoutLeft->addChild(optionsChoice);

	// EngineSampleRateChoice *srChoice = new EngineSampleRateChoice();
	// srChoice->box.size.x = 100;
	// layoutLeft->addChild(srChoice);

	wireOpacitySlider = new Slider();
	wireOpacitySlider->box.size.x = 150;
	wireOpacitySlider->label = "Cable opacity";
	wireOpacitySlider->precision = 0;
	wireOpacitySlider->unit = "%";
	wireOpacitySlider->setLimits(0.0, 100.0);
	wireOpacitySlider->setDefaultValue(50.0);
	layoutLeft->addChild(wireOpacitySlider);

	wireTensionSlider = new Slider();
	wireTensionSlider->box.size.x = 150;
	wireTensionSlider->label = "Cable tension";
	wireTensionSlider->unit = "";
	wireTensionSlider->setLimits(0.0, 1.0);
	wireTensionSlider->setDefaultValue(0.5);
	layoutLeft->addChild(wireTensionSlider);

	struct ZoomSlider : Slider {
		void onAction(EventAction &e) override {
			Slider::onAction(e);
			gRackScene->zoomWidget->setZoom(roundf(value) / 100.0);
			gRackScene->scrollWidget->updateForOffsetChange();
		}
	};
	zoomSlider = new ZoomSlider();
	zoomSlider->box.size.x = 150;
	zoomSlider->precision = 0;
	zoomSlider->label = "Zoom";
	zoomSlider->unit = "%";
	zoomSlider->setLimits(25.0, 200.0);
	zoomSlider->setDefaultValue(100.0);
	layoutLeft->addChild(zoomSlider);

	struct AddModuleButton : Button {
		void onAction(EventAction &e) override {
			appModuleBrowserCreate();
		}
	};

	auto addModuleButton = new AddModuleButton();
	addModuleButton->text = "Add Module";
	addModuleButton->box.size.x = 100;
	layoutLeft->addChild(addModuleButton);

#ifdef TOUCH
	layoutRight = new SequentialLayout();
	layoutRight->padding = Vec(5, 5);
	layoutRight->spacing = 5;
	layoutRight->alignment = SequentialLayout::RIGHT_ALIGNMENT;
	addChild(layoutRight);

	struct RMBButton : Button {
		void draw(NVGcontext *vg) override {
			bndToolButton(vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_ALL, (gForceRMB ? BND_ACTIVE : state), -1, text.c_str());
		}

		void onAction(EventAction &e) override {
			//TODO: in reality at the moment it can only activate the mode
			gForceRMB = !gForceRMB;
		}
	};

	auto rmbButton = new RMBButton();
	rmbButton->text = "RMB";
	rmbButton->box.size.x = BND_WIDGET_HEIGHT*2.5;
	layoutRight->addChild(rmbButton);
#endif

/*
	cpuUsageButton = new RadioButton();
	cpuUsageButton->box.size.x = 100;
	cpuUsageButton->label = "CPU usage";
	layoutLeft->addChild(cpuUsageButton);
*/

/*#if defined(RELEASE)
	Widget *pluginManager = new PluginManagerWidget();
	layoutLeft->addChild(pluginManager);
#endif*/
}

void Toolbar::draw(NVGcontext *vg) {
	bndBackground(vg, 0.0, 0.0, box.size.x, box.size.y);
	bndBevel(vg, 0.0, 0.0, box.size.x, box.size.y);

	Widget::draw(vg);
}

void Toolbar::onResize() {
#ifdef TOUCH
	layoutRight->box.size.x = box.size.x;
#endif
}

} // namespace rack
