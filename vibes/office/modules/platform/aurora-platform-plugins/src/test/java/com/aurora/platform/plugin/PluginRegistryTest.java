package com.aurora.platform.plugin;

import org.junit.jupiter.api.Test;

import java.util.concurrent.atomic.AtomicBoolean;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

class PluginRegistryTest {
    @Test
    void activatesContributionsAndDeactivatesPlugin() {
        PluginRegistry registry = new PluginRegistry();
        AtomicBoolean deactivated = new AtomicBoolean(false);

        registry.activate(new AuroraPlugin() {
            @Override
            public String id() {
                return "test";
            }

            @Override
            public String name() {
                return "Test";
            }

            @Override
            public void activate(AuroraPluginContext context) {
                context.addMenuAction(new PluginAction("test.menu", "Test Action", () -> {
                }));
            }

            @Override
            public void deactivate() {
                deactivated.set(true);
            }
        });

        assertEquals(1, registry.menuActions().size());
        registry.deactivateAll();
        assertTrue(deactivated.get());
        assertTrue(registry.menuActions().isEmpty());
    }
}
