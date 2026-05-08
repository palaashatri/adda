package com.aetheris.ingestion;

import com.aetheris.shared.proto.GeoEntity;
import io.netty.bootstrap.ServerBootstrap;
import io.netty.channel.*;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.SocketChannel;
import io.netty.channel.socket.nio.NioServerSocketChannel;
import io.netty.handler.codec.LineBasedFrameDecoder;
import io.netty.handler.codec.string.StringDecoder;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Component;
import org.springframework.web.client.RestTemplate;

import jakarta.annotation.PostConstruct;
import jakarta.annotation.PreDestroy;

@Component
public class AdsbIngestionGateway {
    private static final Logger logger = LoggerFactory.getLogger(AdsbIngestionGateway.class);
    private final int port = 30001; // Standard SBS-1 port for ADS-B
    private final AdsbParser parser = new AdsbParser();
    private final RestTemplate restTemplate = new RestTemplate();
    private final String streamingCoreUrl = "http://localhost:8081/internal/ingest/entity";
    
    private EventLoopGroup bossGroup;
    private EventLoopGroup workerGroup;

    @PostConstruct
    public void start() {
        Thread.ofVirtual().start(() -> {
            bossGroup = new NioEventLoopGroup(1);
            workerGroup = new NioEventLoopGroup();
            try {
                ServerBootstrap b = new ServerBootstrap();
                b.group(bossGroup, workerGroup)
                        .channel(NioServerSocketChannel.class)
                        .childHandler(new ChannelInitializer<SocketChannel>() {
                            @Override
                            public void initChannel(SocketChannel ch) {
                                ch.pipeline().addLast(new LineBasedFrameDecoder(1024));
                                ch.pipeline().addLast(new StringDecoder());
                                ch.pipeline().addLast(new AdsbHandler(parser, restTemplate, streamingCoreUrl));
                            }
                        })
                        .option(ChannelOption.SO_BACKLOG, 128)
                        .childOption(ChannelOption.SO_KEEPALIVE, true);

                logger.info("ADS-B Ingestion Gateway starting on port {}", port);
                ChannelFuture f = b.bind(port).sync();
                f.channel().closeFuture().sync();
            } catch (InterruptedException e) {
                logger.error("ADS-B Gateway interrupted", e);
                Thread.currentThread().interrupt();
            } finally {
                stop();
            }
        });
    }

    @PreDestroy
    public void stop() {
        if (bossGroup != null) bossGroup.shutdownGracefully();
        if (workerGroup != null) workerGroup.shutdownGracefully();
    }

    private static class AdsbHandler extends SimpleChannelInboundHandler<String> {
        private final AdsbParser parser;
        private final RestTemplate restTemplate;
        private final String streamingCoreUrl;

        public AdsbHandler(AdsbParser parser, RestTemplate restTemplate, String streamingCoreUrl) {
            this.parser = parser;
            this.restTemplate = restTemplate;
            this.streamingCoreUrl = streamingCoreUrl;
        }

        @Override
        protected void channelRead0(ChannelHandlerContext ctx, String msg) {
            GeoEntity entity = parser.parseSbs1(msg);
            if (entity != null) {
                try {
                    restTemplate.postForEntity(streamingCoreUrl, entity.toByteArray(), Void.class);
                    logger.debug("Forwarded entity: {}", entity.getId());
                } catch (Exception e) {
                    logger.error("Failed to forward entity to streaming-core", e);
                }
            }
        }
    }
}
