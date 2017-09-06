//
//  ViewController.h
//  SpectREM
//
//  Created by Mike Daley on 01/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <SpriteKit/SpriteKit.h>
#import <GameplayKit/GameplayKit.h>

@class EmulationScene;

@interface EmulationViewController : NSViewController

#pragma mark - Properties

@property (assign)  IBOutlet    SKView              *skView;
@property (strong)              EmulationScene      *scene;
@property (weak)    IBOutlet    NSVisualEffectView  *configEffectsView;
@property (weak)    IBOutlet    NSScrollView        *configScrollView;

#pragma mark - Methods

- (void *)getDisplayBuffer;
- (void)loadFileWithURL:(NSURL *)url addToRecent:(BOOL)addToRecent;

@end

