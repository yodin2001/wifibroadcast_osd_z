#!/bin/bash
systemctl stop fpv-video
systemctl stop osd

cp -a build/osd fpv_video/{fpv_video,fpv_video.sh} /usr/bin

cat > /etc/systemd/system/osd.service <<EOF
[Unit]
Description=FPV OSD
After=network-online.target

[Service]
#ExecStart=/bin/openvt -s -e -- bash -c 'TERM=linux setterm --blank force --clear all --cursor off /dev/tty && exec /usr/bin/osd'
ExecStart=/usr/bin/osd
Type=simple
Restart=always
RestartSec=1s
TimeoutStopSec=10s
KillMode=control-group

[Install]
WantedBy=multi-user.target
EOF

cat > /etc/systemd/system/fpv-video.service <<EOF
[Unit]
Description=FPV video
After=network-online.target

[Service]
ExecStart=/usr/bin/fpv_video.sh
Type=simple
Restart=always
RestartSec=1s
TimeoutStopSec=10s
KillMode=control-group

[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
systemctl enable fpv-video
systemctl enable osd
systemctl restart fpv-video
systemctl restart osd