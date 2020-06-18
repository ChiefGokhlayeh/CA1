add_cus_dep('pytxcode', 'pytxmcr', 0, 'pythontex');
sub pythontex { return system("pythontex \"$_[0]\"; touch \"$_[0].pytxmcr\"")}
