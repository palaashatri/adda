package com.aurora.platform.ai;

import java.io.IOException;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.time.Duration;
import java.util.concurrent.Flow;

/**
 * Minimal HTTP provider for user-supplied Ollama and OpenAI-compatible endpoints.
 */
public final class HttpAiProvider implements AiProvider {
    private final String providerId;
    private final String endpoint;
    private final String model;
    private final String apiKey;
    private final boolean ollamaNative;
    private final HttpClient client;

    public HttpAiProvider(String providerId, String endpoint, String model, String apiKey, boolean ollamaNative) {
        this.providerId = providerId == null || providerId.isBlank() ? "http-ai" : providerId;
        this.endpoint = endpoint;
        this.model = model == null || model.isBlank() ? AiModelCatalog.DEFAULT_GEMMA4_MODEL : model;
        this.apiKey = apiKey == null ? "" : apiKey;
        this.ollamaNative = ollamaNative;
        this.client = HttpClient.newBuilder().connectTimeout(Duration.ofSeconds(4)).build();
    }

    @Override
    public String id() {
        return providerId;
    }

    @Override
    public boolean supportsStreaming() {
        return false;
    }

    @Override
    public AiResponse complete(AiRequest request) {
        if (endpoint == null || endpoint.isBlank()) {
            return new AiResponse(id(), "AI endpoint is not configured.", 0, 0);
        }
        try {
            String prompt = request.prompt();
            String body = ollamaNative ? ollamaBody(prompt) : openAiCompatibleBody(prompt);
            HttpRequest.Builder builder = HttpRequest.newBuilder(URI.create(endpoint))
                    .timeout(Duration.ofSeconds(30))
                    .header("Content-Type", "application/json")
                    .POST(HttpRequest.BodyPublishers.ofString(body));
            if (!apiKey.isBlank()) {
                builder.header("Authorization", "Bearer " + apiKey);
            }
            HttpResponse<String> response = client.send(builder.build(), HttpResponse.BodyHandlers.ofString());
            if (response.statusCode() < 200 || response.statusCode() >= 300) {
                return new AiResponse(id(), "AI provider returned HTTP " + response.statusCode(), 0, 0);
            }
            String text = ollamaNative ? extractJsonString(response.body(), "response") : extractJsonString(response.body(), "content");
            return new AiResponse(id(), text.isBlank() ? response.body() : text, tokenEstimate(prompt), tokenEstimate(text));
        } catch (IOException exception) {
            return new AiResponse(id(), "AI provider request failed: " + exception.getMessage(), 0, 0);
        } catch (InterruptedException exception) {
            Thread.currentThread().interrupt();
            return new AiResponse(id(), "AI provider request interrupted.", 0, 0);
        } catch (IllegalArgumentException exception) {
            return new AiResponse(id(), "Invalid AI endpoint: " + exception.getMessage(), 0, 0);
        }
    }

    @Override
    public Flow.Publisher<AiToken> stream(AiRequest request) {
        AiResponse response = complete(request);
        return subscriber -> subscriber.onSubscribe(new Flow.Subscription() {
            private boolean delivered;

            @Override
            public void request(long n) {
                if (delivered || n <= 0) {
                    return;
                }
                delivered = true;
                subscriber.onNext(new AiToken(response.text(), true));
                subscriber.onComplete();
            }

            @Override
            public void cancel() {
                delivered = true;
            }
        });
    }

    private String ollamaBody(String prompt) {
        return "{\"model\":\"" + escapeJson(model) + "\",\"prompt\":\"" + escapeJson(prompt) + "\",\"stream\":false}";
    }

    private String openAiCompatibleBody(String prompt) {
        return "{\"model\":\"" + escapeJson(model)
                + "\",\"messages\":[{\"role\":\"user\",\"content\":\"" + escapeJson(prompt)
                + "\"}],\"stream\":false}";
    }

    private static String extractJsonString(String json, String key) {
        String needle = "\"" + key + "\"";
        int start = json.indexOf(needle);
        if (start < 0) {
            return "";
        }
        int colon = json.indexOf(':', start + needle.length());
        int quote = json.indexOf('"', colon + 1);
        if (colon < 0 || quote < 0) {
            return "";
        }
        int end = findStringEnd(json, quote + 1);
        if (end < 0) {
            return "";
        }
        return unescapeJson(json.substring(quote + 1, end));
    }

    private static int findStringEnd(String text, int start) {
        boolean escaped = false;
        for (int index = start; index < text.length(); index++) {
            char current = text.charAt(index);
            if (escaped) {
                escaped = false;
            } else if (current == '\\') {
                escaped = true;
            } else if (current == '"') {
                return index;
            }
        }
        return -1;
    }

    private static String escapeJson(String value) {
        return value == null ? "" : value
                .replace("\\", "\\\\")
                .replace("\"", "\\\"")
                .replace("\n", "\\n")
                .replace("\r", "\\r")
                .replace("\t", "\\t");
    }

    private static String unescapeJson(String value) {
        StringBuilder builder = new StringBuilder();
        boolean escaped = false;
        for (int index = 0; index < value.length(); index++) {
            char current = value.charAt(index);
            if (!escaped) {
                if (current == '\\') {
                    escaped = true;
                } else {
                    builder.append(current);
                }
                continue;
            }
            switch (current) {
                case '"' -> builder.append('"');
                case '\\' -> builder.append('\\');
                case 'n' -> builder.append('\n');
                case 'r' -> builder.append('\r');
                case 't' -> builder.append('\t');
                default -> builder.append(current);
            }
            escaped = false;
        }
        return builder.toString();
    }

    private static int tokenEstimate(String text) {
        if (text == null || text.isBlank()) {
            return 0;
        }
        return Math.max(1, text.trim().split("\\s+").length);
    }
}
